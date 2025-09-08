//-------------------------------------------------------------------
//
// normalize_test.cpp
//
// Simple test of the Normalize() function of the Waveform class.
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
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <vector>

// From an attenuation level between 0 dB (loudest) and -infinity
// dB (quietest), returns the corresponding linear gain multiplier
// value.
static float dbToLinear(float db)
{
    return powf(10.0f, db / 20.0f);
}

static bool normalize_test_iter(size_t numSamples, size_t numChannels, float dbLevel)
{
    // Generate a random waveform.
    Waveform wav;
    wav.SetRate(22000);
    if (!wav.Populate(numSamples, numChannels))
    {
        printf("Waveform::Populate failed.\n");
        return false;
    }
    float *sample = wav.GetSamplesPtr();
    for (size_t index = 0; index < numSamples * numChannels; index++)
    {
        *sample++ = (static_cast<float>(rand() % 2000) - 1000.0f);
    }

    // Normalize the waveform.
    float linearLevel = dbToLinear(dbLevel);
    float smin_before = wav.GetLowestSample();
    float smax_before = wav.GetHighestSample();
    printf("Test numSamples = %zu, numChannels = %zu, dbLevel = %.3f (linear = %.3f)\n", numSamples, numChannels, dbLevel, linearLevel);
    printf("  Before:  smin = %.2f, smax = %.2f\n", smin_before, smax_before);
    wav.Normalize(dbLevel);

    // See if the max level in the waveform matches the
    // normalization level we asked for.
    float smin = wav.GetLowestSample();
    float smax = wav.GetHighestSample();
    printf("  After:   smin = %.2f, smax = %.2f\n", smin, smax);
    if (smax > linearLevel * 1.001)
    {
        printf("Audio level higher than expected after normalization!\n");
        printf("  Target:  %.4f\n", linearLevel);
        printf("  Actual:  %.4f\n", smax);
        return false;
    }
    if (smax < linearLevel * 0.8)
    {
        printf("Audio level lower than expected after normalization!\n");
        printf("  Target:  %.4f\n", linearLevel);
        printf("  Actual:  %.4f\n", smax);
        return false;
    }

    return true;
}

// Run the normalization tests and return true if successful.
bool test_normalize()
{
    int error_count = 0;

    printf("Starting audio normalization tests.\n");

    for (int testIndex = 0; testIndex < 10; testIndex++)
    {
        printf("Normalization test %d:\n", testIndex);

        size_t numSamples = 100000 + rand() % 100000;
        size_t numChannels = 1 + (rand() & 1);
        float dbLevel = -1.0f -static_cast<float>(testIndex);

        if (!normalize_test_iter(numSamples, numChannels, dbLevel))
            error_count++;
    }

    if (error_count)
    {
        printf("Error count during normalization tests:  %d\n", error_count);
        return false;
    }

    printf("Normalization tests OK.\n");
    return true;
}

