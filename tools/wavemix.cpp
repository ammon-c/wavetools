//-------------------------------------------------------------------
//
// wavemix.cpp
// Program to mix two or more audio files together into one audio
// file.
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

// Name and parameters for one of the audio files to be mixed.
struct InFile
{
    std::wstring m_filename;
    float        m_mixVolume = 0.5;
    float        m_mixStartTimeSeconds = 0.0f;
};

struct ProgramSettings
{
    // Names and info about the audio files to read and write.
    std::vector<InFile> m_inFiles;
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
const wchar_t *program_name = L"WaveVibrato";
static void printname() { printf("%S:  ", program_name); }

//
// Mixes input files together into one output file.
//
static bool MixToAudioFile(const ProgramSettings &settings)
{
    printname();
    printf("Settings:\n");
    printf("  Processing %zu input files to '%S'\n",
        settings.m_inFiles.size(), settings.m_outFilename.c_str());
    printf("  Preferred sample type:  %s\n", settings.m_useFloat ? "float" : "integer");
    printf("  Preferred sample size:  %u\n", settings.m_useBytesPerSample);

    //
    // Load the input files.
    //

    std::vector<Waveform> wavs;
    for (const auto &infile : settings.m_inFiles)
    {
        Waveform wav;
        if (!WaveformLoadFromFile(infile.m_filename.c_str(), wav, nullptr, nullptr))
        {
            printname();
            printf("Failed loading audio data from \"%S\"!\n", infile.m_filename.c_str());
            return false;
        }

        wavs.push_back(wav);
    }

    printname();
    printf("Loaded %zu input file(s).\n", wavs.size());
    fflush(stdout);

    // Determine the min and max number of channels used in
    // all of the input sounds.
    size_t minChannels = 99;
    size_t maxChannels = 0;
    for (const auto &wav : wavs)
    {
        size_t numChannels = wav.GetNumChannels();
        if (numChannels > maxChannels)
            maxChannels = numChannels;
        if (numChannels < minChannels)
            minChannels = numChannels;
    }

    // If the sounds don't have the same number of channels,
    // convert them all to stereo.
    if (minChannels != maxChannels)
    {
        printname();
        printf("Input files have inconsistent number of audio channels.\n");
        printf("Converting all input audio to stereo (2-channel) format.\n");

        for (auto &wav : wavs)
        {
            if (!wav.ConvertToStereo())
            {
                printname();
                printf("Failed converting waveform to stereo!\n");
                return false;
            }
        }
    }

    // Determine the min and max sampling rates used in
    // all of the input sounds.
    unsigned minRate = 999999;
    unsigned maxRate = 0;
    for (const auto &wav : wavs)
    {
        unsigned rate = wav.GetRate();
        if (rate > maxRate)
            maxRate = rate;
        if (rate < minRate)
            minRate = rate;
    }

    // If the files don't have the same sample rate,
    // convert them all to the highest sample rate.
    if (minRate != maxRate)
    {
        printname();
        printf("Input files have inconsistent sampling rates.\n");
        printf("Converting all input audio to %u Hz.\n", maxRate);

        for (auto &wav : wavs)
        {
            if (wav.GetRate() != maxRate)
                continue;

            if (!wav.Resample(maxRate))
            {
                printname();
                printf("Failed resampling waveform to %u Hz!\n", maxRate);
                return false;
            }
        }
    }

    size_t rate = wavs[0].GetRate();
    size_t numChannels = wavs[0].GetNumChannels();

    // Determine the overall length of the mixed output file.
    // Also determines the starting sample number for each
    // of the input files int the output.
    float totalSeconds = 0.0f;
    std::vector<size_t>  startSampleIndexes(wavs.size(), 0);
    std::vector<float *> samplePtrs(wavs.size(), nullptr);
    for (size_t index = 0; index < wavs.size(); index++)
    {
        float thisSeconds = settings.m_inFiles[index].m_mixStartTimeSeconds;
        thisSeconds += wavs[index].GetDurationInSeconds();
        if (thisSeconds > totalSeconds)
            totalSeconds = thisSeconds;

        startSampleIndexes[index] =
            wavs[index].TimeToSampleIndex(settings.m_inFiles[index].m_mixStartTimeSeconds) * numChannels;

        samplePtrs[index] = wavs[index].GetSamplesPtr();
    }

    // Create the empty output waveform.
    Waveform wavOut;
    size_t outNumSamples = static_cast<size_t>(totalSeconds * wavs[0].GetRate());
    wavOut.Populate(outNumSamples, wavs[0].GetNumChannels(), nullptr);
    wavOut.SetRate(static_cast<unsigned>(rate));

    //
    // Mix the input waveforms into the output waveform.
    //
    float *sampleOut = wavOut.GetSamplesPtr();
    for (size_t index = 0; index < outNumSamples * numChannels; index++)
    {
        float value = 0.0f;

        for (size_t iwav = 0; iwav < wavs.size(); iwav++)
        {
            Waveform &wavin = wavs[iwav];
            const InFile &filein = settings.m_inFiles[iwav];

            if (index >= startSampleIndexes[iwav] &&
                index < startSampleIndexes[iwav] + wavin.GetNumSamples() * numChannels)
            {
                float insample = samplePtrs[iwav][index - startSampleIndexes[iwav]];
                value += insample * filein.m_mixVolume;
            }
        }

        value = Waveform::ClipValue(value, -1, 1);
        *sampleOut++ = value;
    }

    //
    // Save the altered waveform to the output file.
    //

    printname();
    printf("Saving %zu samples (%.2f seconds) to '%S' at %u Hz\n",
        wavOut.GetNumSamples(), wavOut.GetDurationInSeconds(),
            settings.m_outFilename.c_str(), wavOut.GetRate());
    fflush(stdout);

    if (!WaveformSaveToFile(settings.m_outFilename.c_str(), wavOut, nullptr, nullptr,
                            settings.m_useFloat, settings.m_useBytesPerSample))
    {
        printname();
        printf("Failed saving audio data to \"%S\"!\n", settings.m_outFilename.c_str());
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
        "Description:  WaveMix mixes multiple audio files together and \n"
        "  writes the mixed audio to a new audio file. \n"
        "\n"
        "Usage:  wavemix [options] outfile infile[,volume[,start]] [infile2...]\n"
        "\n"
        "Where:\n"
        "  outfile : Indicates the name of the file to which the mixed \n"
        "            audio waveform will be written. \n"
        "\n"
        "  infile : Indicates the name of an audio file to be mixed. \n"
        "\n"
        "  volume : Indicates the volume level of 'infile' in the mix. \n"
        "           The level is between 0 and 1.  Default is 0.5. \n"
        "\n"
        "  start : Indicates the time offset in the output file where \n"
        "          'infile' should start being mixed in, in seconds. \n"
        "          Default is 0. \n"
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
        else if (settings.m_outFilename.empty())
        {
            // First non-option argument is the output filename.
            settings.m_outFilename = argv[iarg];
        }
        else
        {
            //
            // Parse input file argument of the form:
            //
            //    filename[,volume[,start]]
            //

            const wchar_t *pChar = &argv[iarg][0];
            InFile infile;
            while (*pChar && *pChar != ',')
                infile.m_filename += *pChar++;

            if (*pChar == ',')
            {
                // Extract the volume parameter.
                pChar++;
                std::wstring volume;
                while (*pChar && *pChar != ',')
                    volume += *pChar++;
                float volumef = static_cast<float>(_wtof(volume.c_str()));
                if (volumef >= 0.0f && volumef <= 1.0f)
                {
                    infile.m_mixVolume = volumef;
                }
                else
                {
                    printname();
                    printf("Volume parameter out of range '%S'\n", argv[iarg]);
                    return false;
                }
            }

            if (*pChar == ',')
            {
                // Extract the starting time parameter.
                pChar++;
                std::wstring start;
                while (*pChar && *pChar != ',')
                    start += *pChar++;
                float startf = static_cast<float>(_wtof(start.c_str()));
                if (startf >= 0.0f && startf < 1000.0f)
                {
                    infile.m_mixStartTimeSeconds = startf;
                }
                else
                {
                    printname();
                    printf("Mix start time parameter out of range '%S'\n", argv[iarg]);
                    return false;
                }
            }

            settings.m_inFiles.push_back(infile);
        }
    }

    if (settings.m_inFiles.empty() || settings.m_outFilename.empty())
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
        if (!MixToAudioFile(settings))
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

