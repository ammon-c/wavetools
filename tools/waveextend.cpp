//-------------------------------------------------------------------
//
// waveextend.cpp
// Program to extend an audio waveform by adding a period of silence.
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

#define INVALID_SAMPLE_INDEX static_cast<size_t>(~0)

struct ProgramSettings
{
    // Names of the audio files to read and write.
    std::wstring m_inFilename;
    std::wstring m_outFilename;

    // How many samples to add to the beginning or ending of
    // the waveform.  If m_UseTime is true, these indicate
    // the number of seconds x 1000 instead of the number of
    // samples.
    size_t m_numExtendBegin = INVALID_SAMPLE_INDEX;
    size_t m_numExtendEnd = INVALID_SAMPLE_INDEX;

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
const wchar_t *program_name = L"WaveExtend";
static void printname() { printf("%S:  ", program_name); }

//
// Adds silence to the beginning and/or end of a waveform.
//
static bool ExtendAudioFile(
        const wchar_t *inFilename,
        const wchar_t *outFilename,
        bool useFloat,
        unsigned useBytesPerSample,
        size_t extendBegin,
        size_t extendEnd,
        bool useTime
        )
{
    printname();
    printf("Settings:\n");
    printf("  Extending '%S' to '%S'\n", inFilename, outFilename);
    if (!useTime)
    {
        if (extendBegin > 0)
            printf("  Extend start of waveform by %zu samples.\n", extendBegin);
        if (extendEnd > 0)
            printf("  Extend end of waveform by %zu samples.\n", extendEnd);
    }
    else
    {
        if (extendBegin > 0)
            printf("  Extend start of waveform by %.2f seconds.\n", static_cast<float>(extendBegin) / 1000.0f);
        if (extendEnd > 0)
            printf("  Extend end of waveform by %.2f seconds.\n", static_cast<float>(extendEnd) / 1000.0f);
    }
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
    // Extend the waveform accordingly.
    //

    if (useTime)
    {
        extendBegin = wav.TimeToSampleIndex(static_cast<float>(extendBegin) / 1000.0f);
        extendEnd   = wav.TimeToSampleIndex(static_cast<float>(extendEnd)   / 1000.0f);
    }

    if (extendBegin > 0)
    {
        printname();
        printf("Inserting %zu samples (%.2f seconds) at beginning of waveform.\n",
            extendBegin, wav.SampleIndexToTime(extendBegin));
        fflush(stdout);

        if (!wav.Insert(0, extendBegin))
        {
            printname();
            printf("Failed inserting %zu samples at beginning of waveform!\n", extendBegin);
            return false;
        }
    }
    if (extendEnd > 0)
    {
        printname();
        printf("Inserting %zu samples (%.2f seconds) at end of waveform.\n",
            extendEnd, wav.SampleIndexToTime(extendEnd));
        fflush(stdout);

        if (!wav.Insert(wav.GetNumSamples(), extendEnd))
        {
            printname();
            printf("Failed inserting %zu samples at end of waveform!\n", extendEnd);
            return false;
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
        "Description:  WaveExtend reads an audio file, adds silence to \n"
        "  the beginning and/or ending of the waveform, and writes the \n"
        "  altered audio waveform to a new file. \n"
        "\n"
        "Usage:  waveextend [options] before after infile outfile\n"
        "\n"
        "Where:\n"
        "  before : Indicates how many samples to add to the beginning \n"
        "       of the waveform.  Or if '-UseTime' is also given, indicates \n"
        "       how many seconds to add to the beginning of the waveform. \n"
        "\n"
        "  after : Indicates how many samples to add to the end of the \n"
        "       waveform.  Or if '-UseTime' is also given, indicates how \n"
        "       many seconds to add to the beginning of the waveform. \n"
        "\n"
        "Options:\n"
        "  -UseTime : Indicates that the 'before' and 'after' parameters \n"
        "       are measured in seconds rather than number of samples. \n"
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
        else if (settings.m_numExtendBegin == INVALID_SAMPLE_INDEX)
        {
            float value = static_cast<float>(_wtof(argv[iarg]));
            settings.m_numExtendBegin = static_cast<size_t>(value * 1000.0f);
            if (settings.m_numExtendBegin < 0.0f)
            {
                printf("Invalid 'before' parameter '%S'\n", argv[iarg]);
                return false;
            }
        }
        else if (settings.m_numExtendEnd == INVALID_SAMPLE_INDEX)
        {
            float value = static_cast<float>(_wtof(argv[iarg]));
            settings.m_numExtendEnd = static_cast<size_t>(value * 1000.0f);
            if (settings.m_numExtendEnd < 0.0f)
            {
                printf("Invalid 'after' parameter '%S'\n", argv[iarg]);
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

    if (settings.m_numExtendBegin == INVALID_SAMPLE_INDEX ||
        settings.m_numExtendEnd == INVALID_SAMPLE_INDEX ||
        settings.m_inFilename.empty() ||
        settings.m_outFilename.empty())
    {
        printname();
        printf("Not enough arguments!\n");
        return false;
    }

    if (!settings.m_useTime)
    {
        settings.m_numExtendBegin /= 1000;
        settings.m_numExtendEnd   /= 1000;
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
        if (!ExtendAudioFile(
                settings.m_inFilename.c_str(),
                settings.m_outFilename.c_str(),
                settings.m_useFloat,
                settings.m_useBytesPerSample,
                settings.m_numExtendBegin,
                settings.m_numExtendEnd,
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

