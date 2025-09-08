//-------------------------------------------------------------------
//
// wavegate.cpp
// Program to silence the near-silent parts of an audio file, aka
// a noise gate filter.
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

const float defaultThreshold = 0.1f;

struct ProgramSettings
{
    // Names of the audio files to read and write.
    std::wstring m_inFilename;
    std::wstring m_outFilename;

    // Portions of the waveform below this amplitude are
    // considered "silent".
    float m_threshold = defaultThreshold;

    bool m_removeLeadingSilence = false;
    bool m_removeTrailingSilence = false;

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
const wchar_t *program_name = L"WaveGate";
static void printname() { printf("%S:  ", program_name); }

// Applies a noise gate filter to the waveform.
// The portions of the waveform below the given threshold level
// are replaced with silence.
static bool NoiseGateFilter(
    Waveform &wav,
    float threshold,
    bool removeLeading,
    bool removeTrailing
    )
{
    const size_t numQuietSamplesForSilence =
                    wav.GetRate() * wav.GetNumChannels() / 5;
    size_t numSamples = wav.GetNumSamples() * wav.GetNumChannels();
    size_t numChannels = wav.GetNumChannels();
    size_t isample = 0;
    float *samples = wav.GetSamplesPtr();
    size_t firstSilentSample = 0;
    size_t firstSilentCount = 0;
    size_t lastSilentSample = 0;
    size_t lastSilentCount = 0;
    while (isample < numSamples)
    {
        if (fabsf(samples[isample]) < threshold)
        {
            // This sample is quiet.  Determine how many quiet
            // samples in a row we have.
            size_t qcount = 1;
            isample++;
            while ((isample + qcount < numSamples) &&
                   fabsf(samples[isample + qcount]) < threshold)
            {
                qcount++;
            }

            // If there were enough quiet samples in a row,
            // replace them with silence.
            if (qcount >= numQuietSamplesForSilence)
            {
                printname();
                printf("Silencing %zu samples at %zu.\n", qcount / numChannels, isample / numChannels);
                wav.Silence(isample / numChannels, qcount / numChannels, true);

                // Keep track of the first and last silent chunks.
                lastSilentSample = isample;
                lastSilentCount = qcount;
                if (firstSilentCount == 0)
                {
                    firstSilentSample = lastSilentSample;
                    firstSilentCount = lastSilentCount;
                }
            }

            isample += qcount;
        }
        else
        {
            // This sample is not quiet, so skip samples
            // until we run out or find another quiet sample.
            while ((isample < numSamples) &&
                fabsf(samples[isample]) >= threshold)
            {
                isample++;
            }
        }
    }

    // Check if the last silence was within 10 samples of the end
    // of the waveform.  We allow 10 samples of junk at the end
    // because many waveforms contain a short pop/click in the
    // last few samples of the waveform.
    numChannels = wav.GetNumChannels();
    numSamples = wav.GetNumSamples();
    if (removeTrailing &&
        firstSilentSample != lastSilentSample &&
        lastSilentCount > 0 &&
        lastSilentSample + lastSilentCount >= numSamples * numChannels - 10)
    {
        // Delete the silence at the end of the waveform.
        printname();
        printf("Deleting %zu samples of silence from end of waveform.\n",
            lastSilentCount / numChannels);
        wav.Delete(lastSilentSample / numChannels, lastSilentCount / numChannels);
    }

    // Check if the first silence was within 10 samples of the start
    // of the waveform.  We allow 10 samples of junk at the beginning
    // because many waveforms contain a short pop/click in the first
    // few samples of the waveform.
    if (removeLeading &&
        firstSilentCount > 0 &&
        firstSilentSample < 10)
    {
        // Delete the silence at the start of the waveform.
        printname();
        printf("Deleting %zu samples of silence from start of waveform.\n",
            firstSilentCount / numChannels);
        wav.Delete(firstSilentSample / numChannels, firstSilentCount / numChannels);
    }

    return true;
}

//
// Reads samples from an audio file and writes them to another
// audio file, possibly with a different sample type or sample
// size.
//
static bool ApplyNoiseGateFilterToAudioFile(
        const wchar_t *inFilename,
        const wchar_t *outFilename,
        bool useFloat,
        unsigned useBytesPerSample,
        float threshold,
        bool removeLeadingSilence,
        bool removeTrailingSilence
        )
{
    printname();
    printf("Settings:\n");
    printf("  Filtering '%S' to '%S' with gate threshold %G\n", inFilename, outFilename, threshold);
    printf("  Trim beginning silence:  %s\n", removeLeadingSilence ? "Yes" : "No");
    printf("  Trim ending silence:     %s\n", removeTrailingSilence ? "Yes" : "No");
    printf("  Preferred sample type:   %s\n", useFloat ? "float" : "integer");
    printf("  Preferred sample size:   %u\n", useBytesPerSample);

    //
    // Read the audio from the input file.
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
    float hiSample = wav.GetHighestSample();
    float loSample = wav.GetLowestSample();
    printf("Sample range:  low=%G  hi=%G\n", loSample, hiSample);
    float maxSample = fabsf(hiSample) > fabsf(loSample) ? fabsf(hiSample) : fabsf(loSample);
    float adjustedThreshold = threshold * maxSample;
    if (adjustedThreshold != threshold)
    {
        printname();
        printf("Adjusted threshold %G\n", adjustedThreshold);
    }

    //
    // Filter the audio.
    //

    printname();
    printf("Applying gate filter with threshold %G\n", adjustedThreshold);
    fflush(stdout);

    if (!NoiseGateFilter(wav, adjustedThreshold,
                         removeLeadingSilence, removeTrailingSilence))
    {
        printname();
        printf("Error during gate filter!\n");
        return false;
    }

    //
    // Save the filtered waveform to the output file.
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
        "Description:  WaveGate reads an audio file, applies a noise \n"
        "  gate filter to the audio waveform, and writes the altered \n"
        "  waveform to a new file. \n"
        "\n"
        "Usage:  wavegate [options] infile outfile\n"
        "\n"
        "Options:\n"
        "  -Threshold=x : Specifies the gate threshold, where 'x' is a \n"
        "       sample level between 0.0000001 and 1.  Default is %G. \n"
        "\n"
        "  -TrimStart : Removes the silence at the beginning of the \n"
        "       waveform, if any. \n"
        "\n"
        "  -TrimEnd : Removes the silence at the end of the waveform, \n"
        "       if any. \n"
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
        "\n",
        defaultThreshold);
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
            else if (OptionNameIs(argv[iarg], L"Threshold"))
            {
                settings.m_threshold = static_cast<float>(_wtof(OptionValue(argv[iarg])));
                if (settings.m_threshold < 0.0000001 || settings.m_threshold >= 1.0f)
                {
                    printname();
                    printf("Invalid threshold parameter %G!\n", settings.m_threshold);
                    return false;
                }
            }
            else if (OptionNameIs(argv[iarg], L"TrimStart"))
            {
                settings.m_removeLeadingSilence = true;
            }
            else if (OptionNameIs(argv[iarg], L"TrimEnd"))
            {
                settings.m_removeTrailingSilence = true;
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
    ProgramSettings settings;
    if (!ParseCommandLineArguments(argc, argv, settings))
        return EXIT_FAILURE;    

    try
    {
        if (!ApplyNoiseGateFilterToAudioFile(
                settings.m_inFilename.c_str(),
                settings.m_outFilename.c_str(),
                settings.m_useFloat,
                settings.m_useBytesPerSample,
                settings.m_threshold,
                settings.m_removeLeadingSilence,
                settings.m_removeTrailingSilence))
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

