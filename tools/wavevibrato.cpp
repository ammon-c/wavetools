//-------------------------------------------------------------------
//
// wavevibrato.cpp
// Program to apply a vibrato effect to an audio file.  By adjusting
// the parameters, a variety of vibrato-based effects (like flanging,
// phasing, and tape warbling) can be achieved. 
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

    // Vibrato effect parameters.
    float m_vibratoWidthSeconds = 1.0f;
    float m_vibratoDepthMs = 50.0f;
    float m_wetLevel = 1.0f;
    float m_dryLevel = 0.0f;

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
const wchar_t *program_name = L"WaveVibrato";
static void printname() { printf("%S:  ", program_name); }

//
// Adds vibrato effect to an audio file.
//
static bool AddVibratoToAudioFile(
        const wchar_t *inFilename,
        const wchar_t *outFilename,
        bool useFloat,
        unsigned useBytesPerSample,
        float vibratoWidthSeconds,
        float vibratoDepthMs,
        float wetLevel,
        float dryLevel
        )
{
    printname();
    printf("Settings:\n");
    printf("  Processing '%S' to '%S' with vibrato width %.2f seconds, depth %.2f ms\n",
        inFilename, outFilename, vibratoWidthSeconds, vibratoDepthMs);
    printf("  Levels:  wet:%.2f  dry:%.2f\n", wetLevel, dryLevel);
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

    //
    // Apply vibrato to the waveform's samples.
    //

    size_t vibratoWidthSamples = wav.TimeToSampleIndex(vibratoWidthSeconds) * numChannels;
    size_t vibratoDepthSamples = wav.TimeToSampleIndex(vibratoDepthMs / 1000.0f / 4.0f) * numChannels;
    printname();
    printf("Samples per vibrato cycle:  %zu\n", vibratoWidthSamples);
    printname();
    printf("Vibrato depth in samples:   %zu\n", vibratoDepthSamples);
    fflush(stdout);

    Waveform wavOut = wav;
    float *samplesIn = wav.GetSamplesPtr();
    float *sample = wavOut.GetSamplesPtr();
    for (size_t index = 0; index < numSamples * numChannels; index++)
    {
        float posInCycle0to1 = static_cast<float>(index % vibratoWidthSamples) /
                                    vibratoWidthSamples;
        float posMult0to1 = sinf(posInCycle0to1 * 3.1415927f * 2.0f);

        size_t inIndex = index + static_cast<size_t>(posMult0to1 * vibratoDepthSamples);
        if (inIndex > (numSamples * numChannels))
        {
            inIndex = index;
        }

#if 0 // Useful for debugging.

        printf("DEBUG: index = %zu, inIndex = %zu, posInCycle = %.2f\n",
            index, inIndex, posInCycle0to1);

#endif

        float value = samplesIn[inIndex] * wetLevel;
        if (dryLevel > 0)
        {
            value += samplesIn[index] * dryLevel;
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

    if (!WaveformSaveToFile(outFilename, wavOut, nullptr, nullptr,
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
        "Description:  WaveVibrato reads an audio file, applies a vibrato \n"
        "  effect to the waveform, and writes the altered waveform to a \n"
        "  new audio file.  By changing the values of the width, depth, \n"
        "  drylevel, and wetlevel parameters, a variety of vibrato-based \n"
        "  effects can be produced; for example, flanging, phasing, and \n"
        "  tape flutter. \n"
        "\n"
        "Usage:  wavevibrato [options] width depth infile outfile\n"
        "\n"
        "Where:\n"
        "  width : Indicates the width of the vibrato effect in seconds \n"
        "          per cycle. \n"
        "\n"
        "  depth : Indicates the depth of the vibrato effect in milliseconds. \n"
        "\n"
        "Options:\n"
        "  -WetLevel=x : Specify how much wet signal to include in the \n"
        "       altered waveform, as a floating-point number between 0 \n"
        "       and 1.  Default is 1.\n"
        "\n"
        "  -DryLevel=x : Specify how much dry signal to include in the \n"
        "       altered waveform, as a floating-point number between 0 \n"
        "       and 1.  Default is 0.\n"
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
            else if (OptionNameIs(argv[iarg], L"WetLevel"))
            {
                settings.m_wetLevel = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                if (settings.m_wetLevel < 0 || settings.m_wetLevel > 1)
                {
                    printname();
                    printf("Invalid WetLevel parameter value '%S'.\n", argv[iarg]);
                    return false;
                }
            }
            else if (OptionNameIs(argv[iarg], L"DryLevel"))
            {
                settings.m_dryLevel = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                if (settings.m_dryLevel < 0 || settings.m_dryLevel > 1)
                {
                    printname();
                    printf("Invalid DryLevel parameter value '%S'.\n", argv[iarg]);
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
                settings.m_vibratoWidthSeconds = static_cast<float>(_wtof(argv[iarg]));
                if (settings.m_vibratoWidthSeconds <= 0.0f || settings.m_vibratoWidthSeconds > 100.0f)
                {
                    printf("Invalid value for width parameter, '%S'\n", argv[iarg]);
                    return false;
                }
            }
            else if (nonopts == 2)
            {
                settings.m_vibratoDepthMs = static_cast<float>(_wtof(argv[iarg]));
                if (settings.m_vibratoDepthMs <= 0.0f || settings.m_vibratoDepthMs > 10000.0f)
                {
                    printf("Invalid value for depth parameter, '%S'\n", argv[iarg]);
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
            else if (nonopts > 4)
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
        if (!AddVibratoToAudioFile(
                settings.m_inFilename.c_str(),
                settings.m_outFilename.c_str(),
                settings.m_useFloat,
                settings.m_useBytesPerSample,
                settings.m_vibratoWidthSeconds,
                settings.m_vibratoDepthMs,
                settings.m_wetLevel,
                settings.m_dryLevel))
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

