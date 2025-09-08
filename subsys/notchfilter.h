//-------------------------------------------------------------------
//
// notchfilter.h
//
// Simple C++ class for a notch audio filter.
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

// Class to help apply a notch filter to a series of audio
// samples using a second order infinite impulse response (IIR)
// style filter.
class NotchFilter
{
public:
    // Construct the notch filter.
    //      sampleRate : Indicates the sample rate of the audio
    //                   data that will be filtered, in Hertz.
    //      centerFrequenty : Indicates the center frequency
    //                        of the notch in Hertz.
    //      qFactor : Specifies the Q-factor for the notch bandwidth.
    //                Typical values would be less than one for a
    //                wide notch covering many frequencies, up to more
    //                than ten for a narrow notch to remove a specific
    //                frequency from the audio signal. 
    NotchFilter(float sampleRate, float centerFrequency, float qFactor)
        : m_sampleRate(sampleRate), m_centerFrequency(centerFrequency),
          m_qFactor(qFactor)
    {
        CalculateCoefficients();

        // Initialize history for IIR filter
        m_x_prev1 = 0.0;
        m_x_prev2 = 0.0;
        m_y_prev1 = 0.0;
        m_y_prev2 = 0.0;
    }

    // Applies the notch filter to one audio sample, returning the
    // filtered sample value.
    float FilterSample(float inputSample)
    {
        float outputSample =
                m_b0 * inputSample +
                m_b1 * m_x_prev1 +
                m_b2 * m_x_prev2 -
                m_a1 * m_y_prev1 -
                m_a2 * m_y_prev2;

        // Update history
        m_x_prev2 = m_x_prev1;
        m_x_prev1 = inputSample;
        m_y_prev2 = m_y_prev1;
        m_y_prev1 = outputSample;

        return outputSample;
    }

private:
    const float pi = 3.1415927f;
    float m_sampleRate;
    float m_centerFrequency;
    float m_qFactor;

    float m_b0, m_b1, m_b2, m_a0, m_a1, m_a2;           // Filter coefficients.
    float m_x_prev1, m_x_prev2, m_y_prev1, m_y_prev2;   // History for IIR filter.

    void CalculateCoefficients()
    {
        float omega0 = 2.0f * pi * m_centerFrequency / m_sampleRate;
        float alpha = std::sin(omega0) / (2.0f * m_qFactor);

        m_b0 = 1.0f;
        m_b1 = -2.0f * std::cosf(omega0);
        m_b2 = 1.0f;
        m_a0 = 1.0f + alpha; // Note a0 is often 1 in standard IIR forms, but useful for clarity.
        m_a1 = -2.0f * std::cosf(omega0);
        m_a2 = 1.0f - alpha;

        // Normalize coefficients by a0.
        m_b0 /= m_a0;
        m_b1 /= m_a0;
        m_b2 /= m_a0;
        m_a1 /= m_a0;
        m_a2 /= m_a0;
    }
};

