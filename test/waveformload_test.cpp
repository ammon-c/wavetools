//-------------------------------------------------------------------
//
// waveform_load_test.cpp
//
// Simple test of the waveformload.cpp module.  Given the name of an
// audio file this attempts to read the audio data from the file into
// a Waveform object.
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

#include "waveform.h"
#include "waveformload.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <vector>

bool test_waveform_load(wchar_t *filename)
{
    printf("Starting Waveform load test with '%S'\n", filename);
    fflush(stdout);

    // Attempt to load the file.
    Waveform wav;
    bool result = WaveformLoadFromFile(filename, wav);

    // Check that the load succeeded.
    if (!result)
    {
        printf("Failed loading '%S'\n", filename);
        return false;
    }

    // Check that the audio data is non-empty.
    if (wav.GetNumSamples() < 1 || wav.GetRate() < 1)
    {
        printf("Invalid data reading '%S'\n", filename);
        return false;
    }

    printf("Success loading '%S' into Waveform object.\n", filename);
    return true;
}

