//-------------------------------------------------------------------
//
// wavereverb.cpp
// Program to apply a reverb effect to an audio file.
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
#include "../subsys/highpass.h"
#include "../subsys/lowpass.h"
#include "../subsys/notchfilter.h"
#include "../subsys/bandpassfilter.h"
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

    // Reverb effect parameters.
    float m_dwell = 0.3f;       // 0.1 small room to 1.0f auditorium.
    float m_dryLevel = 0.7f;
    float m_wetLevel = 0.5f;

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
const wchar_t *program_name = L"WaveReverb";
static void printname() { printf("%S:  ", program_name); }

static bool ApplyHighPassFilter(Waveform &wav, float highPassFreq)
{
    printname();
    printf("  Applying high pass filter %.2f Hz\n", highPassFreq);

    HighPassFilter filt(highPassFreq, static_cast<float>(wav.GetRate()));

    float smin = wav.GetLowestSample();
    float smax = wav.GetHighestSample();
    printname();
    printf("    Sample levels range input:  min=%.2f  max=%.2f\n", smin, smax);

    float *sample = wav.GetSamplesPtr();
    const size_t numSamples = wav.GetNumSamples();
    const size_t numChannels = wav.GetNumChannels();
    for (size_t index = 0; index < numSamples * numChannels; index++)
    {
        float value = *sample;
        value = filt.FilterSample(value);
        value = Waveform::ClipValue(value, -1, 1);
        *sample++ = value;
    }

    smin = wav.GetLowestSample();
    smax = wav.GetHighestSample();
    printname();
    printf("    Sample levels range after:  min=%.2f  max=%.2f\n", smin, smax);

    return true;
}

static bool ApplyLowPassFilter(Waveform &wav, float lowPassFreq)
{
    printname();
    printf("  Applying low pass filter %.2f Hz\n", lowPassFreq);

    LowPassFilter filt(lowPassFreq, static_cast<float>(wav.GetRate()));

    float smin = wav.GetLowestSample();
    float smax = wav.GetHighestSample();
    printname();
    printf("    Sample levels range input:  min=%.2f  max=%.2f\n", smin, smax);

    float *sample = wav.GetSamplesPtr();
    const size_t numSamples = wav.GetNumSamples();
    const size_t numChannels = wav.GetNumChannels();
    for (size_t index = 0; index < numSamples * numChannels; index++)
    {
        float value = *sample;
        value = filt.FilterSample(value);
        value = Waveform::ClipValue(value, -1, 1);
        *sample++ = value;
    }

    smin = wav.GetLowestSample();
    smax = wav.GetHighestSample();
    printname();
    printf("    Sample levels range after:  min=%.2f  max=%.2f\n", smin, smax);

    return true;
}

static bool ApplyNotchFilter(Waveform &wav, float notchFreq, float notchQ)
{
    printname();
    printf("  Applying notch filter %.2f Hz @ %.2f Q-factor\n", notchFreq, notchQ);

    NotchFilter filt(static_cast<float>(wav.GetRate()), notchFreq, notchQ);

    float *sample = wav.GetSamplesPtr();
    const size_t numSamples = wav.GetNumSamples();
    const size_t numChannels = wav.GetNumChannels();
    for (size_t index = 0; index < numSamples * numChannels; index++)
    {
        float value = *sample;
        value = filt.FilterSample(value);
        value = Waveform::ClipValue(value, -1, 1);
        *sample++ = value;
    }

    return true;
}

static bool ApplyBandpassFilter(Waveform &wav, float bandpassFreq, float bandpassQ)
{
    printname();
    printf("  Applying bandpass filter %.2f Hz @ %.2f Q-factor\n", bandpassFreq, bandpassQ);

    BandpassFilter filt(static_cast<float>(wav.GetRate()), bandpassFreq, bandpassQ);

    float *sample = wav.GetSamplesPtr();
    const size_t numSamples = wav.GetNumSamples();
    const size_t numChannels = wav.GetNumChannels();
    for (size_t index = 0; index < numSamples * numChannels; index++)
    {
        float value = *sample;
        value = filt.FilterSample(value);
        value = Waveform::ClipValue(value, -1, 1);
        *sample++ = value;
    }

    return true;
}

// Fills wavOut with a delayed version of wavIn.
static bool ApplyDelay(const Waveform &wavIn, Waveform &wavOut, float delayMs, float level = 1.0f)
{
    // Make the output the same size as the input.
    wavOut.Populate(wavIn.GetNumSamples(), wavIn.GetNumChannels());
    wavOut.SetRate(wavIn.GetRate());

    const size_t numSamples = wavIn.GetNumSamples();
    const size_t numChannels = wavIn.GetNumChannels();
    size_t delayOffsetSamples = static_cast<size_t>(wavIn.GetRate() * delayMs / 1000.0f) * numChannels;
    printname();
    printf("  Delay time in samples:  %zu\n", delayOffsetSamples);
    const float *sampleIn = wavIn.GetSamplesPtr();
    float *sampleOut = wavOut.GetSamplesPtr() + delayOffsetSamples;
    for (size_t index = 0; index < numSamples * numChannels - delayOffsetSamples; index++)
    {
        float value = *(sampleIn++) * level;
        value = Waveform::ClipValue(value, -1, 1);
        *sampleOut++ = value;
    }

    float smin = wavIn.GetLowestSample();
    float smax = wavIn.GetHighestSample();
    printname();
    printf("  Sample levels range input:  min=%.2f  max=%.2f\n", smin, smax);
    smin = wavOut.GetLowestSample();
    smax = wavOut.GetHighestSample();
    printname();
    printf("  Sample levels range after:  min=%.2f  max=%.2f\n", smin, smax);

    return true;
}

// Creates a delayed and filtered version of the given input waveform,
// and mixes the result into the given output waveform.
static bool ApplyFilteredDelay(
    const Waveform &wavIn, Waveform &wavOut,
    float delayMs,                          // Delay time in milliseconds.
    float level,                            // Mix level 0..1
    float highPassFreq,                     // If nonzero, high-pass filter frequency in Hz.
    float lowPassFreq,                      // If nonzero, low-pass filter frequency in Hz.
    float notchFreq, float notchQ,          // If nonzero, parameters for notch filter.
    float bandpassFreq, float bandpassQ)    // If nonzero, parameters for bandpass filter.
{
    // Create a blank waveform the same size as the input waveform.
    Waveform wavDelayed;
    wavDelayed.Populate(wavIn.GetNumSamples(), wavIn.GetNumChannels());
    wavDelayed.SetRate(wavIn.GetRate());

    // Fill the blank waveform with a delayed version of the input waveform.
    if (!ApplyDelay(wavIn, wavDelayed, delayMs, level))
    {
        printf("Failed applying delay!\n");
        return false;
    }

    // Apply EQ to the waveform's samples.
    if (highPassFreq > 0.0f)
    {
        if (!ApplyHighPassFilter(wavDelayed, highPassFreq))
        {
            printf("Failed applying high pass filter!\n");
            return false;
        }
    }
    if (lowPassFreq > 0.0f)
    {
        if (!ApplyLowPassFilter(wavDelayed, lowPassFreq))
        {
            printf("Failed applying low pass filter!\n");
            return false;
        }
    }
    if (notchFreq > 0.0f)
    {
        if (!ApplyNotchFilter(wavDelayed, notchFreq, notchQ))
        {
            printf("Failed applying notch filter!\n");
            return false;
        }
    }
    if (bandpassFreq > 0.0f)
    {
        if (!ApplyBandpassFilter(wavDelayed, bandpassFreq, bandpassQ))
        {
            printf("Failed applying bandpass filter!\n");
            return false;
        }
    }

    // Mix the intermediate waveform into the output waveform.
    const float *sampleIn = wavDelayed.GetSamplesPtr();
    float *sampleOut = wavOut.GetSamplesPtr();
    size_t numSamples = wavOut.GetNumSamples();
    size_t numChannels = wavOut.GetNumChannels();
    for (size_t index = 0; index < numSamples * numChannels; index++)
    {
        float value = *sampleOut;
        value += *(sampleIn++) * level;
        value = Waveform::ClipValue(value, -1, 1);
        *sampleOut++ = value;
    }

    return true;
}

//
// Adds reverb effect to an audio file.
//
static bool AddReverbToAudioFile(const ProgramSettings &settings)
{
    printname();
    printf("Settings:\n");
    printf("  Processing '%S' to '%S' with dwell %.2f\n",
        settings.m_inFilename.c_str(), settings.m_outFilename.c_str(),
        settings.m_dwell);
    printf("  Levels:  dry:%.2f  wet:%.2f\n",
        settings.m_dryLevel, settings.m_wetLevel);
    printf("  Preferred sample type:  %s\n", settings.m_useFloat ? "float" : "integer");
    printf("  Preferred sample size:  %u\n", settings.m_useBytesPerSample);

    //
    // Load the input file.
    //

    Waveform wavIn;
    if (!WaveformLoadFromFile(settings.m_inFilename.c_str(), wavIn, nullptr, nullptr))
    {
        printname();
        printf("Failed loading audio data from \"%S\"!\n", settings.m_inFilename.c_str());
        return false;
    }

    printname();
    printf("Loaded %zu samples (%.2f seconds) from '%S' at %u Hz\n",
        wavIn.GetNumSamples(), wavIn.GetDurationInSeconds(),
        settings.m_inFilename.c_str(), wavIn.GetRate());
    fflush(stdout);

    // Table of filtered echoes to add to the waveform to create reverb.
    // This is a very simplistic method of generating reverb.  It is
    // easy to implement, but subjectively doesn't sound as good as
    // other far more complex reverb algorithms, particularly with
    // higher dwell values (larger room sizes). 
    struct
    {
        float m_dwellMult;
        float m_level;
        float m_highPassFreq;
        float m_lowPassFreq;
        float m_notchFreq;
        float m_notchQ;
        float m_bandpassFreq;
        float m_bandpassQ;
    } echoes[] =
    {
        {  613.0f,   0.35f,   500,   2000,   0,      0,      0,      0       },
        {  437.0f,   0.55f,   700,   3000,   0,      0,      0,      0       },
        {  311.0f,   0.65f,  1000,   5000,   0,      0,      0,      0       },
        {  207.0f,   0.75f,   700,   6000,   0,      0,      0,      0       },
        {  133.0f,   0.65f,   500,   4000,   0,      0,      0,      0       },
        {   77.0f,   0.75f,   300,   2000,   0,      0,      0,      0       },
        {   41.0f,   0.65f,   200,   5000,   0,      0,      0,      0       },
        {   23.0f,   0.35f,   200,   6000,   0,      0,      0,      0       },
        {   11.0f,   0.15f,   200,   4000,   0,      0,      0,      0       },
        {  770.0f,   0.35f,     0,      0,   0,      0,   2500,      2       },
        {  510.0f,   0.55f,     0,      0,   0,      0,   3000,      3       },
        {  370.0f,   0.65f,     0,      0,   0,      0,   4000,      4       },
        {  233.0f,   0.75f,     0,      0,   0,      0,   5000,      5       },
        {  177.0f,   0.65f,     0,      0,   0,      0,   4500,      4       },
        {   97.0f,   0.75f,     0,      0,   0,      0,   3500,      5       },
        {   53.0f,   0.65f,     0,      0,   0,      0,   4500,      4       },
        {   31.0f,   0.35f,     0,      0,   0,      0,   2500,      3       },
        {   17.0f,   0.15f,     0,      0,   0,      0,   3000,      2       },

        { 0, 0, 0, 0, 0, 0, 0, 0 }
    };

    //
    // Apply reverb to the waveform's samples.
    //
    Waveform wavOut = wavIn;
    for (int iecho = 0; echoes[iecho].m_dwellMult > 0; iecho++)
    {
        printname();
        printf("Applying reverberation %d\n", iecho);
        fflush(stdout);

        ApplyFilteredDelay(wavIn, wavOut,
            settings.m_dwell * echoes[iecho].m_dwellMult,
            echoes[iecho].m_level,
            echoes[iecho].m_highPassFreq,
            echoes[iecho].m_lowPassFreq,
            echoes[iecho].m_notchFreq, echoes[iecho].m_notchQ,
            echoes[iecho].m_bandpassFreq, echoes[iecho].m_bandpassQ);
    }

    //
    // Save the altered waveform to the output file.
    //

    printname();
    printf("Saving %zu samples (%.2f seconds) to '%S' at %u Hz\n",
        wavOut.GetNumSamples(), wavOut.GetDurationInSeconds(),
        settings.m_outFilename.c_str(), wavOut.GetRate());
    fflush(stdout);

    if (!WaveformSaveToFile(settings.m_outFilename.c_str(), wavOut,
                            nullptr, nullptr,
                            settings.m_useFloat, settings.m_useBytesPerSample))
    {
        printname();
        printf("Failed saving audio data to \"%S\"!\n",
            settings.m_outFilename.c_str());
        return false;
    }

    printname();
    printf("Saved '%S'\n", settings.m_outFilename.c_str());
    fflush(stdout);

    return true;
}

static void PrintUsage()
{
    printf(g_notice_thisispartof);
    printf(g_notice_copyright_short);
    printf(
        "\n"
        "Description:  WaveReverb reads an audio file, applies a reverb \n"
        "  effect to the waveform, and writes the altered waveform to a \n"
        "  new audio file. \n"
        "\n"
        "Usage:  wavereverb [options] dwell infile outfile\n"
        "\n"
        "Where:\n"
        "  dwell : Indicates the room size, from 0.1 to 1.0 \n"
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
                settings.m_dwell = static_cast<float>(_wtof(argv[iarg]));
                if (settings.m_dwell <= 0.0f || settings.m_dwell > 1.0f)
                {
                    printf("Invalid value for dwell parameter, '%S'\n", argv[iarg]);
                    return false;
                }
            }
            else if (nonopts == 2)
            {
                settings.m_inFilename = argv[iarg];
            }
            else if (nonopts == 3)
            {
                settings.m_outFilename = argv[iarg];
            }
            else if (nonopts > 3)
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
        if (!AddReverbToAudioFile(settings))
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

