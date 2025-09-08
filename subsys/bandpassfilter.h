//-------------------------------------------------------------------
//
// bandpassfilter.h
//
// Simple C++ class for a bandpass audio filter.
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

#pragma once
#include <vector>
#include <cmath>

// Class to help apply bandpass filtering to an audio waveform.
// Uses a simple biquad algorithm.
class BandpassFilter
{
public:
    BandpassFilter(float sampleRate, float centerFreq, float Q) :
        m_a0(0), m_a1(0), m_a2(0),
        m_b0(0), m_b1(0), m_b2(0),
        m_z1(0), m_z2(0)
    {
        // Calculate coefficients for bandpass.
        float omega = 2.0f * pi * centerFreq / sampleRate;
        float alpha = std::sinf(omega) / (2.0f * Q);

        m_b0 = alpha;
        m_b1 = 0.0f;
        m_b2 = -alpha;
        m_a0 = 1.0f + alpha;
        m_a1 = -2.0f * std::cosf(omega);
        m_a2 = 1.0f - alpha;

        // Normalize coefficients.
        m_b0 /= m_a0;
        m_b1 /= m_a0;
        m_b2 /= m_a0;
        m_a1 /= m_a0;
        m_a2 /= m_a0;
    }

    float FilterSample(float inputSample)
    {
        float output = m_b0 * inputSample +
                       m_b1 * m_z1 +
                       m_b2 * m_z2 -
                       m_a1 * m_z1 -
                       m_a2 * m_z2;

        m_z2 = m_z1;
        m_z1 = output;

        return output;
    }

private:
    const float pi = 3.1415927f;
    float  m_a0, m_a1, m_a2, m_b0, m_b1, m_b2; // Filter coefficients.
    float  m_z1, m_z2;                         // Previous outputs.
};

