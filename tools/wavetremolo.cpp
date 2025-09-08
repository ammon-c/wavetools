//-------------------------------------------------------------------
//
// wavetremolo.cpp
// Program to add tremolo effect to an audio waveform.
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

    // Width of each tremolo cycle, measured in samples
    // (or measured in seconds x 1000 if m_useTime is true).
    size_t m_width = 0;

    // Depth of each tremolo cycle, between 0 and 1.
    float m_depth = 0;

    // False if parameters are given in number of samples.
    // True if parameters are given in seconds.
    bool m_useTime = false;

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
const wchar_t *program_name = L"WaveTremolo";
static void printname() { printf("%S:  ", program_name); }

//
// Apply tremolo effect to waveform.
//
static bool ApplyTremoloEffectToAudioFile(
        const wchar_t *inFilename,
        const wchar_t *outFilename,
        bool useFloat,
        unsigned useBytesPerSample,
        size_t width,
        float depth,
        bool useTime
        )
{
    printname();
    printf("Settings:\n");
    printf("  Read from '%S' and write to '%S'\n", inFilename, outFilename);
    if (!useTime)
        printf("  Tremolo width:  %zu samples\n", width);
    else
        printf("  Tremolo width:  %G seconds\n", static_cast<float>(width) / 1000.0f);
    printf("  Tremolo depth:  %G\n", depth);
    printf("  Preferred sample type:  %s\n", useFloat ? "float" : "integer");
    printf("  Preferred sample size:  %u\n", useBytesPerSample);

    //
    // Load the input file.
    //

    Waveform wav;
    if (!WaveformLoadFromFile(inFilename, wav, nullptr, nullptr))
    {
        printname();
        printf("Failed loading audio data from \"%S\"!\n", inFilename);
        return false;
    }

    printname();
    printf("Loaded %zu samples (%.2f seconds) from '%S' at %u Hz\n",
        wav.GetNumSamples(), wav.GetDurationInSeconds(), inFilename, wav.GetRate());
    fflush(stdout);

    //
    // Apply the tremolo effect.
    //

    if (useTime)
        width = static_cast<size_t>(wav.TimeToSampleIndex(width / 1000.0f));

    printname();
    printf("Applying tremolo effect, width %zu samples (%G seconds).\n", width, wav.SampleIndexToTime(width));
    fflush(stdout);

    size_t numSamples = wav.GetNumSamples();
    size_t numChannels = wav.GetNumChannels();
    float *sample = wav.GetSamplesPtr();
    for (size_t isample = 0; isample < numSamples; isample++)
    {
        size_t trempos = isample % width;
        float tremAmplitude = 0.0f;
        if (trempos < width / 2)
        {
            tremAmplitude = depth * trempos / (width / 2);
        }
        else
        {
            trempos -= width / 2;
            tremAmplitude = depth * ((width / 2) - 1 - trempos) / (width / 2);
        }

        for (size_t ichannel = 0; ichannel < numChannels; ichannel++)
        {
            *sample++ *= (1.0f - tremAmplitude);
        }
    }

    //
    // Save the altered waveform to the output file.
    //

    printname();
    printf("Saving %zu samples (%.2f seconds) to '%S' at %u Hz\n",
        wav.GetNumSamples(), wav.GetDurationInSeconds(), outFilename, wav.GetRate());
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
        "Description:  WaveTremolo reads an audio file, applies a \n"
        "  tremolo filter to the waveform, and writes the altered \n"
        "  waveform to a new audio file. \n"
        "\n"
        "Usage:  wavetremolo [options] width depth infile outfile\n"
        "\n"
        "Where:\n"
        "  width : Indicates the width of each tremolo pulsation in \n"
        "       samples.  Or if '-useTime' is also given, indicates the \n"
        "       width in seconds. \n"
        "\n"
        "  depth : Indicates the depth of the tremolo effect, from 0 to 1.\n"
        "\n"
        "Options:\n"
        "  -UseTime : Indicates that the 'width' parameter is measured \n"
        "       in seconds rather than number of samples. \n"
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
            else if (OptionNameIs(argv[iarg], L"UseTime"))
            {
                settings.m_useTime = true;
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
        else if (settings.m_width == 0)
        {
            float value = static_cast<float>(_wtof(argv[iarg]));
            settings.m_width = static_cast<size_t>(value * 1000.0f);
            if (settings.m_width < 1 || settings.m_width > 1e10f)
            {
                printf("Invalid width parameter '%S'\n", argv[iarg]);
                return false;
            }
        }
        else if (settings.m_depth == 0.0f)
        {
            settings.m_depth = static_cast<float>(_wtof(argv[iarg]));
            if (settings.m_depth <= 0.0f || settings.m_depth > 1.0f)
            {
                printf("Invalid depth parameter '%S'\n", argv[iarg]);
                return false;
            }
        }
        else if (settings.m_inFilename.empty())
        {
            // Second non-option argument is the input filename.
            settings.m_inFilename = argv[iarg];
        }
        else if (settings.m_outFilename.empty())
        {
            // Third non-option argument is the output filename.
            settings.m_outFilename = argv[iarg];
        }
        else
        {
            printname();
            printf("Too many arguments! (\"%S\")\n", argv[iarg]);
            return false;
        }
    }

    if (settings.m_depth <= 0.0f ||
        settings.m_width < 1 ||
        settings.m_inFilename.empty() ||
        settings.m_outFilename.empty())
    {
        printname();
        printf("Not enough arguments!\n");
        return false;
    }

    if (!settings.m_useTime)
    {
        settings.m_width /= 1000;
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
        if (!ApplyTremoloEffectToAudioFile(
                settings.m_inFilename.c_str(),
                settings.m_outFilename.c_str(),
                settings.m_useFloat,
                settings.m_useBytesPerSample,
                settings.m_width,
                settings.m_depth,
                settings.m_useTime))
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
    printf("Completed OK.\n");
    return EXIT_SUCCESS;
}

