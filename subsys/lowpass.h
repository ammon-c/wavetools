//-------------------------------------------------------------------
//
// lowpass.h
//
// Simple C++ class for a low-pass audio filter.
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

// Class to help apply a low-pass filter to a series of audio samples.
class LowPassFilter
{
public:
    ~LowPassFilter() = default;
    LowPassFilter() = delete;
    LowPassFilter(float cutoff_frequency, float sample_rate)
    {
        // Calculate the smoothing factor (alpha).
        //
        // alpha = dt / (RC + dt) where dt = 1/sample_rate, RC = 1 / (2 * pi * cutoff_frequency)
        // Simplified: alpha = 1 / (1 + (sample_rate / (2 * M_PI * cutoff_frequency)))
        // M_PI is from <cmath>
        float RC = 1.0f / (2.0f * pi * cutoff_frequency);
        float dt = 1.0f / sample_rate;
        m_alpha = dt / (RC + dt);
        
        // Initialize previous output to 0 or the first input sample.
        m_previous_output = 0.0; 
    }

    // Applies filter to a single audio sample.
    // Returns the filtered sample value.
    float FilterSample(float input_sample)
    {
        float output_sample = m_alpha * input_sample + (1.0f - m_alpha) * m_previous_output;
        m_previous_output = output_sample;
        return output_sample;
    }

private:
    const float pi = 3.1415927f;
    float m_alpha = 0.0;
    float m_previous_output = 0.0;
};

