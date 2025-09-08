//-------------------------------------------------------------------
//
// highpass.h
//
// Simple C++ class for a high-pass audio filter.
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

#include <string>
#include <vector>
#include <numeric>
#include <cmath>
#pragma once

// Class to help apply a high-pass filter to a series of audio samples.
class HighPassFilter
{
public:
    ~HighPassFilter() = default;
    HighPassFilter() = delete;
    HighPassFilter(float cutoffFrequency, float sampleRate) :
        m_cutoffFrequency(cutoffFrequency),
        m_sampleRate(sampleRate),
        m_prevInput(0.0f),
        m_prevOutput(0.0f)
    {
        // Calculate alpha coefficient.
        m_alpha = 1.0f / (1.0f + 2.0f * pi * m_cutoffFrequency / m_sampleRate);
    }

    // Applies filter to a single audio sample.
    // Returns the filtered sample value.
    float FilterSample(float inputSample)
    {
        float outputSample = m_alpha * (m_prevOutput + inputSample - m_prevInput);
        m_prevInput = inputSample;
        m_prevOutput = outputSample;
        return outputSample;
    }

private:
    const float pi = 3.1415927f;
    float m_cutoffFrequency = 0.0f;
    float m_sampleRate = 0.0f;
    float m_alpha = 0.0f;
    float m_prevInput = 0.0f;
    float m_prevOutput = 0.0f;
};

