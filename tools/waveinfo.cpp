//-------------------------------------------------------------------
//
// waveinfo.cpp
// Program to print some general information about audio files.
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
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <wchar.h>
#include <vector>

// Print the program name prefix to stdout.
const wchar_t *program_name = L"WaveInfo";
static void printname() { printf("%S:  ", program_name); }

// Prints a time duration to the console in a consistent format,
// showing the elapsed hours, minutes, and seconds.
void print_duration(float seconds)
{
    int mhour = static_cast<int>(seconds / (60 * 60));
    seconds -= mhour * (60 * 60);
    int mmin = static_cast<int>(seconds / 60);
    seconds -= mmin * 60;

    if (mhour)
        printf("%dh:", mhour);

    if (mmin)
        printf(mhour ? "%02dm:" : "%dm:", mmin);

    printf((mhour || mmin) ? "%05.2fds" : "%.2fs", seconds);
}

// Prints information about the given audio file to stdout.
bool process_audio_file(const wchar_t *filename)
{
    printname();
    printf("Processing '%S'\n", filename);

    Waveform wav;
    if (!WaveformLoadFromFile(filename, wav, nullptr, nullptr))
    {
        printname();
        printf("Failed loading audio data from \"%S\"\n", filename);
        return false;
    }

    printf("Waveform information:\n");
    printf("  Samples:     %zu\n", wav.GetNumSamples());
    printf("  Rate:        %u Hz\n", wav.GetRate());
    printf("  Channels:    %zu\n", wav.GetNumChannels());
    printf("  Duration:    %.2f seconds\n", wav.GetDurationInSeconds());
    printf("  FPCM Bytes:  %zu\n", wav.GetTotalBytes());
    fflush(stdout);

    float high = wav.GetHighestSample();
    float low = wav.GetLowestSample();

    printf("  Highest sample:  %8.2f\n", high);
    printf("  Lowest sample:   %8.2f\n", low);
    fflush(stdout);

    return true;
}

static void PrintUsage()
{
    printf(g_notice_thisispartof);
    printf(g_notice_copyright_short);
    printf(
        "\n"
        "Description:  WaveInfo shows general information about one or \n"
        "  more audio files. \n"
        "\n"
        "Usage:  waveinfo [options] file1.wav [file2.wav ...]\n"
        "\n"
        "Options:\n"
        "  -Help : Print this usage information to the console.\n"
        "\n"
        "  -License : Print the copyright notice and software license \n"
        "             information to the console.\n"
        );
}

// Application entry point.
int wmain(int argc, wchar_t **argv)
{
    // Make sure the user gave us at least one argument.
    if (argc < 2)
    {
        PrintUsage();
        return EXIT_FAILURE;
    }

    unsigned error_count = 0;
    try
    {
        // Process each WAV file that was given on the command line.
        for (int iarg = 1; iarg < argc; iarg++)
        {
            if (_wcsicmp(argv[iarg], L"-Help") == 0)
            {
                PrintUsage();
                return EXIT_SUCCESS;
            }
            else if (_wcsicmp(argv[iarg], L"-License") == 0)
            {
                printf(g_notice_copyright_long);
                return EXIT_SUCCESS;
            }
            else if (!process_audio_file(argv[iarg]))
            {
                printname();
                printf("One or more error(s) processing %S!\n", argv[iarg]);
                ++error_count;
            }
        }
    }
    catch(...)
    {
        printname();
        printf("Unexpected program exception!\n");
        ++error_count;
    }

    if (error_count)
    {
        printname();
        printf("Exiting with %u error(s)!\n", error_count);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

