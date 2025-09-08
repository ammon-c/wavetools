//-------------------------------------------------------------------
//
// wavetrim.cpp
// Program to delete part of an audio file, writing the altered
// waveform to a new audio file.
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
#include "waveformsave.h"
#include "cmdopt.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <wchar.h>
#include <ctype.h>
#include <vector>
#include <string>

#define START_AT_END static_cast<size_t>(~0)

struct ProgramSettings
{
    // Names of the audio files to read and write.
    std::wstring m_inFilename;
    std::wstring m_outFilename;

    // For file formats that support both integer and
    // floating-point samples, this indicates which to use
    // when writing the output file.
    bool m_useFloat = false;

    // For file formats that support multiple sample sizes,
    // this indicates which sample size to use when writing
    // 'outfile', where 'x' is typically 1, 2, or 4 for
    // integer samples, and 4 or 8 for floating-point
    // samples.
    unsigned m_useBytesPerSample = 2;

    // Index of the sample to start trimming at.
    // This may be equal to START_AT_END if we should start
    // trimming backward from the end of the waveform.
    size_t m_startSample = 0;

    // The number of samples to be trimmed.
    // If this is zero, samples will be deleted from the
    // starting position (see above) to the end of the
    // waveform. 
    size_t m_numSamples = 0;

    // If this flag is true, the values in m_startSample
    // and m_numSamples are measured in seconds x 1000
    // rather than sample index numbers within the
    // waveform.
    bool m_useTime = false;

    // If this flag is true, then instead of deleting
    // the indicated part of the waveform, we delete
    // everything *except* the indicated part.
    bool m_invert = false;
};

// Print the program name prefix to stdout.
const wchar_t *program_name = L"WaveTrim";
static void printname() { printf("%S:  ", program_name); }

//
// Reads the audio samples from the input file, deletes
// some samples starting at 'startSample', and writes
// the altered waveform to the output file.
//
// If 'numSamples' is zero, it will delete from startSample
// to the end of the waveform.
//
// Note if 'useTime' is true, then 'startSample' and
// 'numSamples' are measured in seconds x 1000 rather than
// sample numbers.
//
// If 'invert' is true, then instead of deleting the indicated
// part of the waveform, we delete everything *except* the
// indicated part.
//
// Returns true if successful.
//
static bool TrimAudioFile(
        const wchar_t *inFilename,
        const wchar_t *outFilename,
        bool useFloat,
        unsigned useBytesPerSample,
        size_t startSample,
        size_t numSamples,
        bool useTime,
        bool invert
        )
{
    printf("---\n");
    printf("Trimming  '%S' to '%S'\n", inFilename, outFilename);
    printf("Preferred sample type:  %s\n", useFloat ? "float" : "integer");
    printf("Preferred sample size:  %u\n", useBytesPerSample);
    if (startSample == START_AT_END)
    {
        if (useTime)
            printf("Trimming %.2f seconds from end of waveform.\n", static_cast<float>(numSamples) / 1000.0f);
        else
            printf("Trimming %zu samples from end of waveform.\n", numSamples);
    }
    else
    {
        if (useTime)
            printf("Trimming %.2f seconds starting at %.2f.\n", static_cast<float>(numSamples) / 1000.0f, static_cast<float>(startSample) / 1000.0f);
        else
            printf("Trimming %zu samples starting at %zu.\n", numSamples, startSample);
    }
    printf("The trimmed portion is to be %s, the rest %s.\n",
        invert ? "KEPT" : "DELETED",
        invert ? "DELETED" : "KEPT");
    printf("---\n");

    //
    // Load the input file.
    //

    Waveform wav;
    if (!WaveformLoadFromFile(inFilename, wav, nullptr, nullptr))
    {
        printf("Failed loading audio data from \"%S\"\n", inFilename);
        return false;
    }

    printf("Loaded %zu samples from '%S' at %u Hz\n", wav.GetNumSamples(), inFilename, wav.GetRate());
    fflush(stdout);

    //
    // Make any necessary adjustments to starting position and count.
    //

    if (useTime && startSample != START_AT_END)
        startSample = wav.TimeToSampleIndex(static_cast<float>(startSample) / 1000.0f);
    if (useTime && numSamples > 0)
        numSamples = wav.TimeToSampleIndex(static_cast<float>(numSamples) / 1000.0f);

    size_t numSamplesInFile = wav.GetNumSamples();
    if (startSample == START_AT_END)
        startSample = numSamplesInFile - numSamples;
    if (startSample >= numSamplesInFile)
    {
        printf("Starting sample %zu is out of range.\n", startSample);
        return false;
    }
    if (numSamples == 0)
        numSamples = wav.GetNumSamples() - startSample;

    //
    // Delete the specified section of the waveform.
    //

    if (!invert)
    {
        printf("Deleting %zu samples starting at %zu.\n", numSamples, startSample);
        fflush(stdout);
    
        if (!wav.Delete(startSample, numSamples))
        {
            printf("Failed deleting samples from waveform.\n");
            return false;
        }
    }
    else
    {
        // Delete the portion before the trim area.
        if (startSample > 0)
        {
            printf("Deleting %zu samples from beginning of waveform.\n", startSample);
            fflush(stdout);

            if (!wav.Delete(0, startSample))
            {
                printf("Failed deleting samples from beginning of waveform.\n");
                return false;
            }
        }

        // Delete the portion after the trim area.
        if (wav.GetNumSamples() > numSamples)
        {
            size_t numToDelete = wav.GetNumSamples() - numSamples;

            printf("Deleting %zu samples from end of waveform.\n", numToDelete);
            fflush(stdout);

            if (!wav.Delete(numSamples, numToDelete))
            {
                printf("Failed deleting samples from end of waveform.\n");
                return false;
            }
        }
    }

    //
    // Save the altered waveform to the output file.
    //

    printf("Saving %zu samples to '%S' at %u Hz\n", wav.GetNumSamples(), outFilename, wav.GetRate());
    fflush(stdout);

    if (!WaveformSaveToFile(outFilename, wav, nullptr, nullptr,
                            useFloat, useBytesPerSample))
    {
        printf("Failed saving audio data to \"%S\"\n", outFilename);
        return false;
    }

    printf("Saved '%S'\n", outFilename);
    fflush(stdout);

    return true;
}

static void PrintUsage()
{
    printf(g_notice_thisispartof);
    printf(g_notice_copyright_short);
    printf(
        "\n"
        "Description:  WaveTrim reads an audio file, deletes a portion \n"
        "  of the audio waveform, and writes the altered waveform to a \n"
        "  new audio file. \n"
        "\n"
        "Usage:  wavetrim [options] infile outfile\n"
        "\n"
        "Options:\n"
        "  -UseTime : Indicates that the numbers used for the -Count \n"
        "       and -Start options are specified in seconds rather than \n"
        "       sample numbers. \n"
        "\n"
        "  -Count=x : Indicates the number of samples to delete.  Or the \n"
        "       number of seconds to delete if -UseTime is also specified. \n"
        "\n"
        "  -Start=x : If 'x' is a number, it indicates the sample number \n"
        "       where deletion will start.  Or the starting position in \n"
        "       seconds if -UseTime is also specified.  If 'x' is the word \n"
        "       END instead of a number, it indicates that deletion will \n"
        "       trim backward from the end of the waveform. \n"
        "\n"
        "  -Invert : Instead of deleting the indicated part of the waveform, \n"
        "       deletes everything except the indicated part. \n"
        "\n"
        "  -Float=x : For file formats that support both integer and \n"
        "       floating-point samples, this indicates which to use \n"
        "       when writing 'outfile', where 'x' may be 'yes' or 'no'. \n"
        "\n"
        "  -BytesPerSample=x : For file formats that support multiple \n"
        "       sample sizes, this indicates which sample size to use \n"
        "       when writing 'outfile', where 'x' is typically 1, 2, or \n"
        "       4 for integer samples, and 4 or 8 for floating-point \n"
        "       samples. \n"
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
            else if (OptionNameIs(argv[iarg], L"Float"))
            {
                wchar_t first = OptionValue(argv[iarg])[0];
                settings.m_useFloat = (first == 'y' || first == 't' || first == '1');
            }
            else if (OptionNameIs(argv[iarg], L"BytesPerSample"))
            {
                settings.m_useBytesPerSample = static_cast<unsigned>(_wtoi(OptionValue(argv[iarg])));
                if (settings.m_useBytesPerSample < 1 || settings.m_useBytesPerSample > 8)
                {
                    printname();
                    printf("Invalid sample size %u.\n", settings.m_useBytesPerSample);
                    return false;
                }
            }
            else if (OptionNameIs(argv[iarg], L"Start"))
            {
                if (_wcsicmp(OptionValue(argv[iarg]), L"END") == 0)
                {
                    settings.m_startSample = START_AT_END;
                }
                else
                {
                    float value = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                    if (value < 0.0f)
                    {
                        printf("Invalid start parameter %f.\n", value);
                        return false;
                    }
                    settings.m_startSample = static_cast<size_t>(value * 1000.0f);
                }
            }
            else if (OptionNameIs(argv[iarg], L"Count"))
            {
                float value = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                if (value < 0.0f)
                {
                    printf("Invalid start parameter %f.\n", value);
                    return false;
                }
                settings.m_numSamples = static_cast<size_t>(value * 1000.0f);
            }
            else if (OptionNameIs(argv[iarg], L"UseTime"))
            {
                settings.m_useTime = true;
            }
            else if (OptionNameIs(argv[iarg], L"Invert"))
            {
                settings.m_invert = true;
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
        else if (settings.m_outFilename.empty())
        {
            settings.m_outFilename = argv[iarg];
        }
        else
        {
            printname();
            printf("Too many arguments! (\"%S\")\n", argv[iarg]);
            return false;
        }
    }

    if (settings.m_inFilename.empty() || settings.m_outFilename.empty())
    {
        printname();
        printf("Not enough arguments!\n");
        return false;
    }

    if (!settings.m_useTime)
    {
        if (settings.m_startSample != START_AT_END)
            settings.m_startSample /= 1000;

        settings.m_numSamples /= 1000;
    }

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
        if (!TrimAudioFile(
                settings.m_inFilename.c_str(),
                settings.m_outFilename.c_str(),
                settings.m_useFloat,
                settings.m_useBytesPerSample,
                settings.m_startSample,
                settings.m_numSamples,
                settings.m_useTime,
                settings.m_invert))
        {
            printname();
            printf("One or more error(s)!\n");
        }
    }
    catch(...)
    {
        printname();
        printf("Unexpected program exception!\n");
        return EXIT_FAILURE;
    }

    printname();
    printf("Completed.\n");
    return EXIT_SUCCESS;
}

