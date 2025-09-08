//-------------------------------------------------------------------
//
// waveeq.cpp
// Program to apply tone controls (aka equalization) to an audio file.
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
#include "lowpass.h"
#include "highpass.h"
#include "notchfilter.h"
#include "bandpassfilter.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <wchar.h>
#include <ctype.h>
#include <string>
#include <vector>

struct ProgramSettings
{
    // Names of the audio files to read and write.
    std::wstring m_inFilename;
    std::wstring m_outFilename;

    // Equalization parameter settings.
    float m_lowPassFreq = 0.0f;
    float m_highPassFreq = 0.0f;
    float m_notchFreq = 0.0f;
    float m_notchQ = 5.0f;
    float m_bandpassFreq = 0.0f;
    float m_bandpassQ = 2.0f;

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
const wchar_t *program_name = L"WaveEQ";
static void printname() { printf("%S:  ", program_name); }

//
// Applies equalization to an audio file.
//
static bool AddEqualizationToAudioFile(
        const wchar_t *inFilename,
        const wchar_t *outFilename,
        bool useFloat,
        unsigned useBytesPerSample,
        float lowPassFreq,  // Low-pass rolloff frequency in Hertz, or zero for no low-pass filter.
        float highPassFreq, // High-pass rolloff frequency in Hertz, or zero for no high-pass filter.
        float bandpassFreq, // Bandpass center frequency in Hertz, or zero for no bandpass filter.
        float bandpassQ,    // Q-factor for bandpass filter.  Ignored if bandpassFreq is zero.
        float notchFreq,    // Notch filter center frequency in Hertz, or zero for no notch filter.
        float notchQ        // Q-factor for notch filter.  Ignored if notchFreq is zero.
        )
{
    printname();
    printf("Settings:\n");
    printf("  Processing '%S' to '%S'\n", inFilename, outFilename);
    if (lowPassFreq > 0.0f)
        printf("    with low pass filter at %.2f Hz rolloff frequency.\n", lowPassFreq);
    if (highPassFreq > 0.0f)
        printf("    with high pass filter at %.2f Hz rolloff frequency.\n", highPassFreq);
    if (bandpassFreq > 0.0f)
        printf("    with bandpass filter centered on %.2f Hz with %.2f Q-factor.\n",
            bandpassFreq, bandpassQ);
    if (notchFreq > 0.0f)
        printf("    with notch filter centered on %.2f Hz with %.2f Q-factor.\n",
            notchFreq, notchQ);
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

    // Create the filtering objects.
    HighPassFilter *pHighPass = nullptr;
    LowPassFilter  *pLowPass  = nullptr;
    NotchFilter    *pNotch    = nullptr;
    BandpassFilter *pBandpass = nullptr;
    if (highPassFreq > 0.0f)
        pHighPass = new HighPassFilter(highPassFreq, static_cast<float>(wav.GetRate()));
    if (lowPassFreq > 0.0f)
        pLowPass = new LowPassFilter(lowPassFreq, static_cast<float>(wav.GetRate()));
    if (bandpassFreq > 0.0f)
        pBandpass = new BandpassFilter(static_cast<float>(wav.GetRate()), bandpassFreq, bandpassQ);
    if (notchFreq > 0.0f)
        pNotch = new NotchFilter(static_cast<float>(wav.GetRate()), notchFreq, notchQ);

    //
    // Apply EQ to the waveform's samples.
    //

    float *sample = wav.GetSamplesPtr();
    for (size_t index = 0; index < numSamples * numChannels; index++)
    {
        float value = *sample;

        if (pLowPass)
            value = pLowPass->FilterSample(value);
        if (pHighPass)
            value = pHighPass->FilterSample(value);
        if (pBandpass)
            value = pBandpass->FilterSample(value);
        if (pNotch)
            value = pNotch->FilterSample(value);

        value = Waveform::ClipValue(value, -1, 1);
        *sample++ = value;
    }

    // Discard the filtering objects.
    if (pLowPass)
        delete pLowPass;
    if (pHighPass)
        delete pHighPass;

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
        "Description:  WaveEQ reads an audo file, applies a low-pass, \n"
        "  high-pass, notch, and/or bandpass filter to the audio signal, \n"
        "  and writes the altered waveform to a new audio file. \n"
        "\n"
        "Usage:  waveeq [options] lowpass highpass infile outfile\n"
        "\n"
        "Where:\n"
        "  lowpass  ..is the low pass (bass cutoff) frequency in Hertz. \n"
        "             Or zero to disable low-pass filtering. \n"
        "  highpass ..is the high pass (treble cutoff) frequency in Hertz. \n"
        "             Or zero to disable high-pass filtering. \n"
        "\n"
        "Options:\n"
        "  -BandPassFreq=x : Applies a bandpass filter with the specified \n"
        "       center frequency. \n"
        "\n"
        "  -BandPassQ=x : Specifies the Q-factor parameter for the bandpass \n"
        "       filter.  Ignored if -BandPassFreq option is not used. \n"
        "\n"
        "  -NotchFreq=x : Applies a notch filter with the specified \n"
        "       center frequency. \n"
        "\n"
        "  -NotchQ=x : Specifies the Q-factor parameter for the notch \n"
        "       filter.  Ignored if -NotchFreq option is not used. \n"
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
            else if (OptionNameIs(argv[iarg], L"BandpassFreq"))
            {
                settings.m_bandpassFreq = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                if (settings.m_bandpassFreq < 1.0f || settings.m_bandpassFreq > 100000.0f)
                {
                    printname();
                    printf("Invalid bandpass frequency parameter '%S'\n", argv[iarg]);
                    return false;
                }
            }
            else if (OptionNameIs(argv[iarg], L"BandpassQ"))
            {
                settings.m_bandpassQ = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                if (settings.m_bandpassQ <= 0.0f || settings.m_bandpassQ > 100.0f)
                {
                    printname();
                    printf("Invalid bandpass Q-factor parameter '%S'\n", argv[iarg]);
                    return false;
                }
            }
            else if (OptionNameIs(argv[iarg], L"NotchFreq"))
            {
                settings.m_notchFreq = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                if (settings.m_notchFreq < 1.0f || settings.m_notchFreq > 100000.0f)
                {
                    printname();
                    printf("Invalid notch filter frequency parameter '%S'\n", argv[iarg]);
                    return false;
                }
            }
            else if (OptionNameIs(argv[iarg], L"NotchQ"))
            {
                settings.m_notchQ = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                if (settings.m_notchQ <= 0.0f || settings.m_notchQ > 100.0f)
                {
                    printname();
                    printf("Invalid notch filter Q-factor parameter '%S'\n", argv[iarg]);
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
                settings.m_lowPassFreq = static_cast<float>(_wtof(argv[iarg]));
                if (settings.m_lowPassFreq < 0.0f || settings.m_lowPassFreq > 50000.0f)
                {
                    printf("Invalid value for low-pass frequency, '%S'\n", argv[iarg]);
                    return false;
                }
            }
            else if (nonopts == 2)
            {
                settings.m_highPassFreq = static_cast<float>(_wtof(argv[iarg]));
                if (settings.m_highPassFreq < 0.0f || settings.m_highPassFreq > 50000.0f)
                {
                    printf("Invalid value for high-pass frequency, '%S'\n", argv[iarg]);
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
        if (!AddEqualizationToAudioFile(
                settings.m_inFilename.c_str(),
                settings.m_outFilename.c_str(),
                settings.m_useFloat,
                settings.m_useBytesPerSample,
                settings.m_lowPassFreq,
                settings.m_highPassFreq,
                settings.m_bandpassFreq,
                settings.m_bandpassQ,
                settings.m_notchFreq,
                settings.m_notchQ))
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

