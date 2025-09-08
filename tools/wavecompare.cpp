//-------------------------------------------------------------------
//
// wavecompare.cpp
// Program to check if two waveforms appear to be the same/similar.
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
    // The names of the two audio files we are to compare.
    std::wstring m_filename1;
    std::wstring m_filename2;

    // If waveforms differ by more than this amount, they
    // are to be considered different waveforms.
    float m_threshold = 0.001f;
};

// Print the program name prefix to stdout.
const wchar_t *program_name = L"WaveCompare";
static void printname() { printf("%S:  ", program_name); }

//
// Compares two audio files, possibly with different sampling
// rates or sample formats, to see if they contain the same or
// substantially similar waveform.  The 'matched' parameter
// will be set to true if the two waveforms appear to be the
// same; otherwise 'matched' is set to false.
//
// Returns false if either file couldn't be read.
//
static bool CompareAudioFiles(
        const wchar_t *filename1,
        const wchar_t *filename2,
        float threshold,
        bool &matched
        )
{
    matched = false;

    printname();
    printf("Settings:\n");
    printf("  Comparing '%S' to '%S' with threshold %.8f\n", filename1, filename2, threshold);

    Waveform wav1;
    if (!WaveformLoadFromFile(filename1, wav1, nullptr, nullptr))
    {
        printname();
        printf("Failed loading audio data from \"%S\"\n", filename1);
        return false;
    }

    printname();
    printf("Loaded '%S', length %.2f seconds.\n", filename1, wav1.GetDurationInSeconds());

    Waveform wav2;
    if (!WaveformLoadFromFile(filename2, wav2, nullptr, nullptr))
    {
        printname();
        printf("Failed loading audio data from \"%S\"\n", filename2);
        return false;
    }

    printname();
    printf("Loaded '%S', length %.2f seconds.\n", filename2, wav2.GetDurationInSeconds());

    // If the waveforms aren't the same length, don't bother
    // comparing them.
    if (fabs(wav1.GetDurationInSeconds() - wav2.GetDurationInSeconds()) > 0.1)
    {
        printname();
        printf("The two waveforms differ in duration (%.2f vs %.2f seconds).\n",
            wav1.GetDurationInSeconds(), wav2.GetDurationInSeconds());
        return true;
    }

    // If the waveforms don't have the same number of channels,
    // don't bother comparing them.
    // TODO: Convert both waveforms to mono instead of refusing to compare them?
    if (wav1.GetNumChannels() != wav2.GetNumChannels())
    {
        printname();
        printf("The two waveforms differ in number of channels (%zu vs %zu)\n",
            wav1.GetNumChannels(), wav2.GetNumChannels());
        return true;
    }

    // TODO: Normalize both waveforms, so volume differences
    // don't impact the comparison.

    // If the waveforms have different sample rates, resample
    // the one with the lower rate.
    if (wav1.GetRate() != wav2.GetRate())
    {
        unsigned rate = wav1.GetRate();
        if (wav2.GetRate() > rate)
            rate = wav2.GetRate();

        printname();
        printf("Sampling rates %u and %u differ.  Upsampling to %u.\n",
            wav1.GetRate(), wav2.GetRate(), rate);

        if (wav1.GetRate() != rate)
            wav1.Resample(rate);
        if (wav2.GetRate() != rate)
            wav2.Resample(rate);
    }

    // Compare the samples in the two waveforms, accumulating
    // the differences as we go.
    float sumDifferences = 0.0f;
    size_t numSamples = wav1.GetNumSamples();
    size_t numChannels = wav1.GetNumChannels();
    if (numSamples > wav2.GetNumSamples())
        numSamples = wav2.GetNumSamples();
    const float *sample1 = wav1.GetSamplesPtr();
    const float *sample2 = wav2.GetSamplesPtr();
    for (size_t isample = 0; isample < numSamples; isample++)
    {
        for (size_t ichannel = 0; ichannel < numChannels; ichannel++)
        {
            float diff = fabsf((*sample1) - (*sample2));
            sumDifferences += diff;
        }

        sample1++;
        sample2++;
    }

    // Output the result of the comparison.
    float avgDifference = sumDifferences / static_cast<float>(numSamples);
    printname();
    printf("Stats:\n");
    printf("  Sum of differences:  %14f\n", sumDifferences);
    printf("  Average difference:  %14f\n", avgDifference);
    printf("  Threshold:           %14f\n", threshold);
    if (avgDifference <= threshold)
        matched = true;

    // No error.
    return true;
}

static void PrintUsage()
{
    printf(g_notice_thisispartof);
    printf(g_notice_copyright_short);
    printf(
        "\n"
        "Description:  WaveCompare compares two audio files to see if \n"
        "  they contain the same or substantially similar waveform. \n"
        "  The program exit code is zero if the two files appear to \n"
        "  match, or one if they don't match or can't be loaded. \n"
        "\n"
        "Usage:  wavecompare [options] file1 file2\n"
        "\n"
        "Options:\n"
        "  -Threshold=x : If the average difference between the samples \n"
        "                 in the two waveforms exceeds value 'x', the \n"
        "                 waveforms are considered to be different from \n"
        "                 each other.  0.001 is the default.\n"
        "\n"
        "  -Help : Print this usage information to the console.\n"
        "\n"
        "  -License : Print the copyright notice and software license \n"
        "             information to the console.\n"
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
            else if (OptionNameIs(argv[iarg], L"Threshold"))
            {
                settings.m_threshold = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                if (settings.m_threshold < 1e-20 || settings.m_threshold > 1.0)
                {
                    printname();
                    printf("Invalid threshold value %.2f.\n", settings.m_threshold);
                    return false;
                }
            }
            else
            {
                printname();
                printf("Unrecognized option '%S'\n", argv[iarg]);
                return false;
            }
        }
        else if (settings.m_filename1.empty())
        {
            settings.m_filename1 = argv[iarg];
        }
        else if (settings.m_filename2.empty())
        {
            settings.m_filename2 = argv[iarg];
        }
        else
        {
            printname();
            printf("Too many arguments! (\"%S\")\n", argv[iarg]);
            return false;
        }
    }

    if (settings.m_filename1.empty() || settings.m_filename2.empty())
    {
        printname();
        printf("Not enough arguments!\n");
        return false;
    }

    return true;
}

//
// Application entry point.
//
// Possible exit codes:
//   0 = The two waveforms appear to be the same.
//   1 = The two waveforms appear to be different.
//   2 = There was an error.  The waveforms couldn't be compared.
//
int wmain(int argc, wchar_t **argv)
{
    ProgramSettings settings;
    if (!ParseCommandLineArguments(argc, argv, settings))
        return 2;

    bool matched = false;
    try
    {
        if (!CompareAudioFiles(settings.m_filename1.c_str(),
                               settings.m_filename2.c_str(),
                               settings.m_threshold,
                               matched))
        {
            printname();
            printf("One or more error(s)!\n");
        }
    }
    catch(...)
    {
        printname();
        printf("Unexpected program exception!\n");
        return 2;
    }

    if (!matched)
    {
        printname();
        printf("RESULT:  No, '%S' and '%S' didn't match.\n",
            settings.m_filename1.c_str(), settings.m_filename2.c_str());
        return 1;
    }

    printname();
    printf("RESULT:  Yes, '%S' and '%S' appear to be a match.\n",
        settings.m_filename1.c_str(), settings.m_filename2.c_str());
    return 0;
}

