//-------------------------------------------------------------------
//
// wavejoin.cpp
// Program to create a new audio file by joining two or more audio
// files together in sequence (one after another).
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

struct ProgramSettings
{
    // Names of the audio files to be joined.
    std::vector<std::wstring> m_filenames;

    // Name of the file to write the joined audio to.
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
};

// Print the program name prefix to stdout.
const wchar_t *program_name = L"WaveJoin";
static void printname() { printf("%S:  ", program_name); }

//
// Combines multiple audio files in sequence (one after another)
// into one long audio file.  Returns true if successful.
//
static bool ConcatenateAudioFiles(
        const std::vector<std::wstring> &filenames,
        const wchar_t *outFilename,
        bool useFloat,
        unsigned useBytesPerSample
        )
{
    printname();
    printf("Settings:\n");
    printf("  Joining ");
    for (const auto &name : filenames)
        printf("\"%S\" ", name.c_str());
    printf(" into \"%S\"\n", outFilename);
    printf("  Preferred sample type:  %s\n", useFloat ? "float" : "integer");
    printf("  Preferred sample size:  %u\n", useBytesPerSample);

    // Load all of the audio files into memory.
    // This isn't ideal, but it's the easiest way to check the
    // sampling rate of all of the files before we decide what
    // rate to use for the output file.
    std::vector<Waveform> wavs;
    for (const auto &name : filenames)
    {
        Waveform wav;
        if (!WaveformLoadFromFile(name.c_str(), wav, nullptr, nullptr))
        {
            printname();
            printf("Failed loading audio data from \"%S\"\n", name.c_str());
            return false;
        }

        wavs.push_back(wav);
    }

    if (wavs.empty())
        return false;

    printname();
    printf("Loaded %zu waveforms.\n", wavs.size());

    // Determine the highest sampling rate of the files we just loaded.
    unsigned rate = 1;
    for (auto &wav : wavs)
    {
        unsigned wrate = wav.GetRate();
        if (wrate > rate)
            rate = wrate;
    }

    // Resample all of the waveforms to the same rate.
    for (auto &wav : wavs)
    {
        if (wav.GetRate() != rate)
        {
            printname();
            printf("Resampling.\n");

            if (!wav.Resample(rate))
            {
                printname();
                printf("Failed resampling waveform!\n");
                return false;
            }
        }
    }

    // Determine the most number of channels used in any of the
    // waveforms we just loaded.
    size_t maxChannels = 0;
    size_t minChannels = 999999;
    for (const auto &wav : wavs)
    {
        size_t num = wav.GetNumChannels();
        if (num > maxChannels)
            maxChannels = num;
        if (num < minChannels)
            minChannels = num;
    }

    // If we have waveforms with differing numbers of channels,
    // convert them all to stereo.
    if (minChannels != maxChannels)
    {
        printname();
        printf("Converting all waveforms to stereo.\n");

        for (auto &wav : wavs)
        {
            if (wav.GetNumChannels() != 2)
                wav.ConvertToStereo();
        }

        maxChannels = 2;
    }

    // Join all of the waveforms into one waveform.
    Waveform outwav;
    outwav.Populate(0, maxChannels);
    outwav.SetRate(rate);
    printname();
    printf("Joining %zu waveforms of %zu channels at %u Hz.\n",
        wavs.size(), maxChannels, outwav.GetRate());
    fflush(stdout);
    for (const auto &wav : wavs)
    {
        size_t numInSamples = wav.GetNumSamples();
        size_t numOutSamples = outwav.GetNumSamples();
        if (numInSamples < 1)
            continue;

        // Make space for the samples we want to add to outwav.
        outwav.Insert(numOutSamples, numInSamples);

        // Copy samples from the input waveform to outwav.
        const float *inSample = wav.GetSamplesPtr();
        float *outSample = outwav.GetSamplesPtr() + numOutSamples * maxChannels;
        for (size_t index = 0; index < numInSamples; index++)
        {
            for (size_t channel = 0; channel < maxChannels; channel++)
            {
                *outSample++ = *inSample++;
            }
        }
    }

    printname();
    printf("Saving %zu samples to '%S' at %u Hz\n",
        outwav.GetNumSamples(), outFilename, outwav.GetRate());
    fflush(stdout);

    if (!WaveformSaveToFile(outFilename, outwav, nullptr, nullptr,
                            useFloat, useBytesPerSample))
    {
        printname();
        printf("Failed saving audio data to \"%S\"!\n", outFilename);
        return false;
    }

    printname();
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
        "Description:  WaveJoin creates a new audio file by joining \n"
        "  multiple audio files together sequentially (one after \n"
        "  another). \n"
        "\n"
        "Usage:  wavejoin [options] infile1 [infile2 ...] outfile\n"
        "\n"
        "Options:\n"
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
            else
            {
                printname();
                printf("Unrecognized option '%S'\n", argv[iarg]);
                return false;
            }
        }
        else
        {
            settings.m_filenames.push_back(argv[iarg]);
        }
    }

    if (settings.m_filenames.size() < 2)
    {
        printname();
        printf("Not enough arguments!\n");
        return false;
    }

    // The last filename from the command line becomes the
    // name of the output file.
    settings.m_outFilename = settings.m_filenames[settings.m_filenames.size() - 1];
    settings.m_filenames.pop_back();

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
        if (!ConcatenateAudioFiles(
                settings.m_filenames,
                settings.m_outFilename.c_str(),
                settings.m_useFloat,
                settings.m_useBytesPerSample))
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

    printname();
    printf("Completed.\n");
    return EXIT_SUCCESS;
}

