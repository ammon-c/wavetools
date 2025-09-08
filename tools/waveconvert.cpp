//-------------------------------------------------------------------
//
// waveconvert.cpp
// Program to read an existing audio file and write the audio data
// to a new audio file in a different format.
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
    // Names of the audio files to read and write.
    std::wstring m_inFilename;
    std::wstring m_outFilename;

    // Specify channel conversion:
    //   0 = If all waveforms have the same number of channels,
    //       leave them as-is, otherwise convert them to stereo.
    //   1 = Convert all waveforms to mono (1 channel).
    //   2 = Convert all waveforms to stereo (2 channels).
    unsigned m_useChannels = 0;

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
const wchar_t *program_name = L"WaveConvert";
static void printname() { printf("%S:  ", program_name); }

//
// Reads samples from an audio file and writes them to another
// audio file, possible with a different sample type or sample
// size.
//
static bool ConvertAudioFile(
        const wchar_t *inFilename,
        const wchar_t *outFilename,
        bool useFloat,
        unsigned useBytesPerSample,
        unsigned useChannels
        )
{
    printname();
    printf("Settings:\n");
    printf("  Converting '%S' to '%S'\n", inFilename, outFilename);
    printf("  Preferred sample type:  %s\n", useFloat ? "float" : "integer");
    printf("  Preferred sample size:  %u\n", useBytesPerSample);
    printf("  Channel format:         ");
    switch(useChannels)
    {
        case 1:     printf("Mono\n");     break;
        case 2:     printf("Stereo\n");   break;
        default:    printf("As-Is\n");    break;
    }

    Waveform wav;
    if (!WaveformLoadFromFile(inFilename, wav, nullptr, nullptr))
    {
        printname();
        printf("Failed loading audio data from \"%S\"!\n", inFilename);
        return false;
    }

    printname();
    printf("Loaded %zu samples (%.2f seconds) from '%S'\n",
        wav.GetNumSamples(), wav.GetDurationInSeconds(), inFilename);
    fflush(stdout);

    // If a channel conversion is needed...
    if (useChannels == 1 && wav.GetNumChannels() != 1)
    {
        printname();
        printf("Converting '%S' to mono.\n", inFilename);

        if (!wav.ConvertToMono())
        {
            printf("Failed converting waveform to mono.\n");
            return false;
        }
    }
    else if (useChannels == 2 && wav.GetNumChannels() != 2)
    {
        printname();
        printf("Converting '%S' to mono.\n", outFilename);

        if (!wav.ConvertToStereo())
        {
            printf("Failed converting waveform to stereo.\n");
            return false;
        }
    }

    //
    // Save the altered waveform to the output file.
    //

    printname();
    printf("Saving %zu samples (%.2f seconds) to '%S'\n",
        wav.GetNumSamples(), wav.GetDurationInSeconds(), outFilename);
    fflush(stdout);

    if (!WaveformSaveToFile(outFilename, wav, nullptr, nullptr,
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
        "Description:  WaveConvert reads an audio file and writes the \n"
        "  waveform data to a new audio file, possibly in a different file \n"
        "  format or with a different sample encoding format. \n"
        "\n"
        "Usage:  waveconvert [options] infile outfile\n"
        "\n"
        "Options:\n"
        "  -Mono : Convert the audio to mono (one channel).\n"
        "\n"
        "  -Stereo : Convert the audio to stereo (two channels).\n"
        "\n"
        "  -Float=x : For file formats that support both integer and \n"
        "       floating-point samples, this indicates which to \n"
        "       use, where 'x' may be 'yes' or 'no'. \n"
        "\n"
        "  -BytesPerSample=x : For file formats that support multiple \n"
        "       sample sizes, this indicates which sample size to use, \n"
        "       where 'x' is typically 1, 2, or 4 for integer samples, \n"
        "       and 4 or 8 for floating-point samples. \n"
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
            else if (OptionNameIs(argv[iarg], L"Mono"))
            {
                settings.m_useChannels = 1;
            }
            else if (OptionNameIs(argv[iarg], L"Stereo"))
            {
                settings.m_useChannels = 2;
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

    return true;
}

// Application entry point.
int wmain(int argc, wchar_t **argv)
{
    // Parse the command line arguments.
    ProgramSettings settings;
    if (!ParseCommandLineArguments(argc, argv, settings))
        return EXIT_FAILURE;

    try
    {
        if (!ConvertAudioFile(
                settings.m_inFilename.c_str(),
                settings.m_outFilename.c_str(),
                settings.m_useFloat,
                settings.m_useBytesPerSample,
                settings.m_useChannels))
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
    printf("Completed OK.\n");
    return EXIT_SUCCESS;
}

