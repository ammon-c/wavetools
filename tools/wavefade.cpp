//-------------------------------------------------------------------
//
// wavefade.cpp
// Program to fade in the beginning of an audio file and/or
// fade out the end of an audio file.
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

    // The duration of fade-in and fade-out in seconds.
    float m_fadeInSeconds = 0.0f;
    float m_fadeOutSeconds = 0.0f;

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
const wchar_t *program_name = L"WaveFade";
static void printname() { printf("%S:  ", program_name); }

//
// Adds fade-in and/or fade-out to an audio file.
// The fadeInSeconds and fadeOutSeconds parameters indicate
// the duration of the fade in seconds.
//
static bool AddFadeInOutToAudioFile(
        const wchar_t *inFilename,
        const wchar_t *outFilename,
        bool useFloat,
        unsigned useBytesPerSample,
        float fadeInSeconds,
        float fadeOutSeconds
        )
{
    printname();
    printf("Settings:\n");
    printf("  Processing '%S' to '%S'\n", inFilename, outFilename);
    if (fadeInSeconds > 0.0f)
        printf("    with up to %.2f seconds of fade-in\n", fadeInSeconds);
    if (fadeOutSeconds > 0.0f)
        printf("    with up to %.2f seconds of fade-out\n", fadeOutSeconds);
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
    size_t numSamples = wav.GetNumSamples();
    size_t numChannels = wav.GetNumChannels();

    printname();
    printf("Loaded %zu samples (%.2f seconds) from '%S' at %u Hz\n",
        wav.GetNumSamples(), wav.GetDurationInSeconds(), inFilename, wav.GetRate());
    fflush(stdout);

    if (fadeInSeconds > wav.GetDurationInSeconds())
        fadeInSeconds = wav.GetDurationInSeconds();
    if (fadeOutSeconds > wav.GetDurationInSeconds())
        fadeOutSeconds = wav.GetDurationInSeconds();

    // Calculate the duration of the fades in samples instead of seconds.
    size_t numFadeInSamples = static_cast<unsigned>(wav.TimeToSampleIndex(fadeInSeconds) * numChannels);
    size_t numFadeOutSamples = static_cast<unsigned>(wav.TimeToSampleIndex(fadeOutSeconds) * numChannels);

    //
    // Apply the fades to the waveform's samples.
    //

    printname();
    printf("Applying %zu samples of fade-in and %zu samples of fade-out to waveform.\n",
        numFadeInSamples / numChannels, numFadeOutSamples / numChannels);
    fflush(stdout);

    size_t fadeOutStart = (numSamples * numChannels) - numFadeOutSamples;
    float *sample = wav.GetSamplesPtr();
    for (size_t index = 0; index < numSamples * numChannels; index++)
    {
        float value = *sample;

        if (index < numFadeInSamples)
        {
            float multiplier = (static_cast<float>(index) / (numFadeInSamples * numChannels));
            value *= multiplier;
        }
        if (index >= fadeOutStart)
        {
            float multiplier = 1.0f - (static_cast<float>(index - fadeOutStart) / (numFadeOutSamples * numChannels));
            value *= multiplier;
        }

        value = Waveform::ClipValue(value, -1, 1);
        *sample++ = value;
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
        "Description:  WaveFade reads an audio file, applies a fade-in to \n"
        "  the beginning of the audio and/or applies a fade-out to the end \n"
        "  of the audio, and writes the altered waveform to a new file. \n"
        "\n"
        "Usage:  wavefade [options] fadein fadeout infile outfile\n"
        "\n"
        "Where:\n"
        "  fadein  : is a number indicating the duration of the fade-in \n"
        "            in seconds.  May be zero for no fade-in. \n"
        "  fadeout : is a number indicating the duration of the fade-out \n"
        "            in seconds.  May be zero for no fade-out. \n"
        "  infile  : is the name of the audio file to read. \n"
        "  outfile : is the name of the file to which the altered audio \n"
        "            will be written. \n"
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

    unsigned nonopts = 0;
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
            nonopts++;
            if (nonopts == 1)
            {
                settings.m_fadeInSeconds = static_cast<float>(_wtof(argv[iarg]));
                if (settings.m_fadeInSeconds <= 0.0f || settings.m_fadeInSeconds > 100.0f)
                {
                    printf("Invalid value for fade-in duration, '%S'\n", argv[iarg]);
                    return false;
                }
            }
            else if (nonopts == 2)
            {
                settings.m_fadeOutSeconds = static_cast<float>(_wtof(argv[iarg]));
                if (settings.m_fadeOutSeconds <= 0.0f || settings.m_fadeOutSeconds > 100.0f)
                {
                    printf("Invalid value for fade-out duration, '%S'\n", argv[iarg]);
                    return false;
                }
            }
            else if (nonopts == 3)
            {
                settings.m_inFilename = argv[iarg];
            }
            else if (nonopts == 4)
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
    ProgramSettings settings;
    if (!ParseCommandLineArguments(argc, argv, settings))
        return EXIT_FAILURE;

    try
    {
        if (!AddFadeInOutToAudioFile(
                settings.m_inFilename.c_str(),
                settings.m_outFilename.c_str(),
                settings.m_useFloat,
                settings.m_useBytesPerSample,
                settings.m_fadeInSeconds,
                settings.m_fadeOutSeconds))
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

