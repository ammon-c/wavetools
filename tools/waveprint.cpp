//-------------------------------------------------------------------
//
// waveprint.cpp
// Program to print a graph of a waveform's amplitude over time to
// the console/terminal.
//
//-------------------------------------------------------------------
//
// (C) Copyright 1994-2025 Ammon R. Campbell.
//
// I wrote this code for use in my own educational and experimental
// programs, but you may also freely use it in yours as long as you
// abide by the following terms and conditions:
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials
//     provided with the distribution.
//   * The name(s) of the author(s) and contributors (if any) may not
//     be used to endorse or promote products derived from this
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.  IN OTHER WORDS, USE AT YOUR OWN RISK, NOT OURS.  
//--------------------------------------------------------------------

#include "notice.h"
#include "waveform.h"
#include "waveformload.h"
#include "cmdopt.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <wchar.h>
#include <ctype.h>
#include <vector>
#include <string>

struct ProgramSettings
{
    // Names of file to be printed.
    std::wstring m_inFilename;

    // If this flag is true, the values in m_startSample,
    // m_numSamples, etc., are measured in seconds x 1000
    // rather than sample index numbers.
    bool m_useTime = false;

    // Index of the first sample to be printed.
    // Or if m_useTime is true, the time of the first sample
    // in seconds x 1000.
    size_t m_startSample = 0;

    // Number of samples to be printed, or zero to print all.
    // Or if m_useTime is true, the time span of samples to
    // be printed in seconds x 1000.
    size_t m_numSamples = 0;

    // Number of samples represented by each line in the
    // printout, or if m_useTime is true, the time span
    // of each line in seconds x 1000.
    size_t m_samplesPerLine = 0;

    // The minimum and maximum amplitude values shown on
    // the graph.  The left edge of the graph is m_yMin,
    // and the right edge of the graph is m_yMax.
    float m_yMin = -1.0f;
    float m_yMax = 1.0f;

    // Width of the printout in characters.
    unsigned m_width = 60;
};

// Print the program name prefix to stdout.
const wchar_t *program_name = L"WavePrint";
static void printname() { printf("%S:  ", program_name); }

// Finds the highest and lowest sample values in the given range
// of samples of the given waveform.  Sets 'lowest' and 'highest'
// accordingly.  The results will both be zero if any parameters
// are out of range.  Note if 'count' is zero, will examine all
// samples from 'start' to the end of the waveform.
static void FindLowestHighestSamplesInRange(
    const Waveform &wav,
    size_t start,
    size_t count,
    float &lowest,
    float &highest)
{
    size_t numSamples = wav.GetNumSamples();
    size_t numChannels = wav.GetNumChannels();

    lowest = highest = 0.0f;
    if (start >= numSamples)
        return;

    if (count == 0 || start + count > numSamples)
        count = numSamples - start;

    lowest = FLT_MAX;
    highest = -FLT_MAX;
    const float *sample = wav.GetSamplesPtr() + start * numChannels;
    for (size_t isample = 0; isample < count; isample++)
    {
        for (size_t ichannel = 0; ichannel < numChannels; ichannel++)
        {
            if (*sample < lowest)
                lowest = *sample;
            if (*sample > highest)
                highest = *sample;

            sample++;
        }
    }

    if (lowest == FLT_MAX || highest == -FLT_MAX)
        lowest = highest = 0.0f;
}

//
// Reads the audio samples from the input file and prints a
// graph of the audio waveform's amplitude over time to the
// console/terminal.
//
// Returns true if successful.
//
static bool PrintAudioFile(
        const wchar_t *inFilename,
        bool useTime,
        size_t startSample,
        size_t numSamples,
        size_t samplesPerLine,
        float yMin,
        float yMax,
        unsigned width
        )
{
    printname();
    printf("Settings:\n");
    printf("  Printing '%S'\n", inFilename);
    if (!useTime)
    {
        printf("  Start at sample:    %zu\n", startSample);
        if (numSamples < 1)
            printf("  Number of samples:  All\n");
        else
            printf("  Number of samples:  %zu\n", numSamples);
        printf("  Samples per line:   %zu\n", samplesPerLine);
    }
    else
    {
        printf("  Start time:         %G seconds\n", static_cast<float>(startSample) / 1000.0f);
        printf("  Time span to print: %G seconds\n", static_cast<float>(numSamples) / 1000.0f);
        printf("  Time span per line: %G seconds\n", static_cast<float>(samplesPerLine) / 1000.0f);
    }
    printf("  Amplitude range:    Min:%G, Max:%G\n", yMin, yMax);
    printf("  Terminal width:     %u characters\n", width);

    //
    // Load the input file.
    //

    Waveform wav;
    if (!WaveformLoadFromFile(inFilename, wav, nullptr, nullptr))
    {
        printf("Failed loading audio data from \"%S\"\n", inFilename);
        return false;
    }

    printname();
    printf("Loaded %zu samples (%G seconds) from '%S' at %u Hz\n",
        wav.GetNumSamples(), wav.GetDurationInSeconds(), inFilename, wav.GetRate());
    fflush(stdout);

    //
    // Adjustments.
    //

    if (useTime)
    {
        startSample    = wav.TimeToSampleIndex(static_cast<float>(startSample) / 1000.0f);
        numSamples     = wav.TimeToSampleIndex(static_cast<float>(numSamples) / 1000.0f);
        samplesPerLine = wav.TimeToSampleIndex(static_cast<float>(samplesPerLine) / 1000.0f);
    }

    size_t numSamplesAll = wav.GetNumSamples();
    if (startSample >= numSamplesAll)
    {
        printname();
        printf("Starting sample %zu out of range!\n", startSample);
        return false;
    }
    if (numSamples == 0)
    {
        // Sample count of zero means print until end of waveform.
        numSamples = numSamplesAll - startSample;
    }
    if (startSample + numSamples > numSamplesAll)
    {
        // Count exceeds available samples, so reduce the count.
        numSamples = numSamplesAll - startSample;
    }

    // Reduce the width to account for the space used by
    // the prefixes on each line of the printout.
    if (width > 11)
        width -= 11;

    //
    // Print the waveform.
    //

    // Print line above header line showing the minimum and
    // maximum amplitude values.
    printf("\n");
    printf("%10G", yMin);
    for (unsigned i = 0; i < width; i++)
        printf(" ");
    printf("%G\n", yMax);

    // Print header line.
    printf("         +");
    for (unsigned x = 0; x < width; x++)
        printf("-");
    printf("+\n");

    // For each line in the printout...
    for (size_t isample = 0; isample < numSamples; isample += samplesPerLine)
    {
        // Determine what chunk of samples is to be printed for this line.
        size_t firstSampleThisLine = startSample + isample;
        size_t numSamplesThisLine = samplesPerLine;
        if (isample + numSamplesThisLine > numSamples)
            numSamplesThisLine = numSamples - isample;

        // Find the lowest and highest samples in this chunk.
        float lowest = 0.0f;
        float highest = 0.0f;
        FindLowestHighestSamplesInRange(wav, firstSampleThisLine,
                            numSamplesThisLine, lowest, highest);
        if (lowest < yMin)
            lowest = yMin;
        if (highest > yMax)
            highest = yMax;

        // Calculate the start and stop positions for the
        // amplitude markings for this line.
        size_t leftCol =  static_cast<size_t>((lowest  - yMin) * width / fabsf(yMax - yMin));
        size_t rightCol = static_cast<size_t>((highest - yMin) * width / fabsf(yMax - yMin));
        if (rightCol >= width)
            rightCol = width - 1;
        if (leftCol > rightCol)
            leftCol = rightCol;

        // Print the sample number of this line.
        printf("%8zu |", firstSampleThisLine);

        // Print the amplitude marks for this line.
        for (size_t i = 0; i < leftCol; i++)
            printf(" ");
        for (size_t i = leftCol; i <= rightCol; i++)
            printf("#");
        for (size_t i = rightCol + 1; i < width; i++)
            printf(" ");
        printf("|\n");
    }

    // Print trailer line.
    printf("         +");
    for (unsigned x = 0; x < width; x++)
        printf("-");
    printf("+\n");

    // Print line below trailer line showing the minimum and
    // maximum amplitude values.
    printf("%10G", yMin);
    for (unsigned i = 0; i < width; i++)
        printf(" ");
    printf("%G\n", yMax);

    printf("\n");

    return true;
}

static void PrintUsage()
{
    printf(g_notice_thisispartof);
    printf(g_notice_copyright_short);
    printf(
        "\n"
        "Description:  WavePrint reads an audio file and prints a graph \n"
        "  of the waveform amplitude over time to the console. \n"
        "\n"
        "Usage:  waveprint [options] filename\n"
        "\n"
        "Options:\n"
        "  -UseTime : Indicates that the numbers used for the -Count, \n"
        "       -Start, and -PerLine options are specified in seconds\n"
        "       rather than sample index numbers. \n"
        "\n"
        "  -Count=x : Indicates the number of samples to print.  Or the \n"
        "       number of seconds to print if -UseTime is also specified. \n"
        "       If -Count is not given, printing will continue until the \n"
        "       end of the waveform. \n"
        "\n"
        "  -Start=x : Indicates the sample number where printing will \n"
        "       start.  Or the starting position in seconds if -UseTime \n"
        "       is also specified. \n"
        "\n"
        "  -PerLine=x : Indicates the number of samples represented by each \n"
        "       line in the printed graph.  Or the number of seconds for \n"
        "       each line of -UseTime is also specified. \n"
        "\n"
        "  -Width=x : Indicates the width of the printed graph in \n"
        "       character cells.  Default is 60.\n"
        "\n"
        "  -Min=x : Indicates the amplitude represented by the left edge \n"
        "       of the graph.  Default is -1.0.\n"
        "\n"
        "  -Max=x : Indicates the amplitude represented by the right edge \n"
        "       of the graph.  Default is 1.0.\n"
        "\n"
        "  -Help : Print this usage information to the console.\n"
        "\n"
        "  -License : Print the copyright notice and software license \n"
        "       information to the console.\n"
        );
}

// Parses the program's command line arguments, placing the
// selected options into the settings structure.  Returns
// true if successful.
static bool ParseCommandLineArguments(
    int argc,
    wchar_t **argv,
    ProgramSettings &settings
    )
{
    if (argc < 2)
    {
        PrintUsage();
        return false;
    }

    for (int iarg = 1; iarg < argc; iarg++)
    {
        if (argv[iarg][0] == '-')
        {
            if (OptionNameIs(argv[iarg], L"Help") || OptionNameIs(argv[iarg], L"?"))
            {
                // The user wants command line help.
                PrintUsage();
                return false;
            }
            else if (OptionNameIs(argv[iarg], L"License"))
            {
                // The user wants the long form copyright and license terms.
                printf(g_notice_copyright_long);
                return false;
            }
            else if (OptionNameIs(argv[iarg], L"Min"))
            {
                float value = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                if (value < -1e10f || value > 1e10f)
                {
                    printname();
                    printf("Invalid Min parameter %f.\n", value);
                    return false;
                }
                settings.m_yMin = value;
            }
            else if (OptionNameIs(argv[iarg], L"Max"))
            {
                float value = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                if (value < -1e10f || value > 1e10f)
                {
                    printname();
                    printf("Invalid Max parameter %f.\n", value);
                    return false;
                }
                settings.m_yMax = value;
            }
            else if (OptionNameIs(argv[iarg], L"Width"))
            {
                settings.m_width = static_cast<unsigned>(_wtoi(OptionValue(argv[iarg])));
                if (settings.m_width < 2 || settings.m_width > 32768)
                {
                    printname();
                    printf("Invalid width parameter %u.\n", settings.m_width);
                    return false;
                }
            }
            else if (OptionNameIs(argv[iarg], L"Start"))
            {
                float value = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                if (value < 0.0f)
                {
                    printname();
                    printf("Invalid start parameter %f.\n", value);
                    return false;
                }
                settings.m_startSample = static_cast<size_t>(value * 1000.0f);
            }
            else if (OptionNameIs(argv[iarg], L"Count"))
            {
                float value = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                if (value < 0.0f)
                {
                    printname();
                    printf("Invalid count parameter %f.\n", value);
                    return false;
                }
                settings.m_numSamples = static_cast<size_t>(value * 1000.0f);
            }
            else if (OptionNameIs(argv[iarg], L"PerLine"))
            {
                float value = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                if (value < 0.0f)
                {
                    printname();
                    printf("Invalid PerLine parameter %f.\n", value);
                    return false;
                }
                settings.m_samplesPerLine = static_cast<size_t>(value * 1000.0f);
            }
            else if (OptionNameIs(argv[iarg], L"UseTime"))
            {
                settings.m_useTime = true;
            }
            else
            {
                printname();
                printf("Unrecognized option '%S'\n", argv[iarg]);
                return false;
            }
        }
        else if (settings.m_inFilename.empty())
        {
            settings.m_inFilename = argv[iarg];
        }
        else
        {
            printname();
            printf("Too many arguments! (\"%S\")\n", argv[iarg]);
            return false;
        }
    }

    if (settings.m_inFilename.empty())
    {
        printname();
        printf("Not enough arguments!\n");
        return false;
    }

    if (!settings.m_useTime)
    {
        settings.m_startSample /= 1000;
        settings.m_numSamples /= 1000;
        settings.m_samplesPerLine /= 1000;
    }

    if (settings.m_samplesPerLine == 0)
        settings.m_samplesPerLine = 10000;

    return true;
}

// Application entry point.
int wmain(int argc, wchar_t **argv)
{
    ProgramSettings settings;
    if (!ParseCommandLineArguments(argc, argv, settings))
        return EXIT_FAILURE;

    try
    {
        if (!PrintAudioFile(
                settings.m_inFilename.c_str(),
                settings.m_useTime,
                settings.m_startSample,
                settings.m_numSamples,
                settings.m_samplesPerLine,
                settings.m_yMin,
                settings.m_yMax,
                settings.m_width))
        {
            printname();
            printf("One or more error(s)!\n");
            return EXIT_FAILURE;
        }
    }
    catch(...)
    {
        printname();
        printf("Unexpected program exception!\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

