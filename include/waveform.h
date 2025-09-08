//-------------------------------------------------------------------
//
// waveform.h
// C++ utility container class for a PCM audio waveform.
//
// NOTES:
//  * The audio signal is represented in memory as an array of
//    32-bit floating-point audio sample values.
//  * The sample values are typically normalized to between
//    -1.0 and +1.0 inclusive.
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
#include <cstdint>

// Container class for a PCM audio waveform.
// Internally we store the audio as an array of floating-point
// sample values between -1.0 and +1.0.  If the audio has more
// than one channel (e.g. stereo), then the samples occur in
// interleaved order in the sample buffer.
class Waveform
{
public:
    Waveform() = default;
    ~Waveform() = default;

    //--------------------------------------------------
    // Initialize
    //--------------------------------------------------

    // Populates this object with an audio signal of the specified
    // duration and number of channels.  If "data" is non-null, the
    // contents of "data" are copied into the Waveform's audio
    // sample buffer in memory; otherwise the audio signal is
    // initialized to silence (all zeroes).
    // Returns true if successful.
    bool Populate(
            size_t numSamples,
            size_t numChannels,
            const float *sampleData = nullptr);

    // Set the waveform's sample rate in Hertz (samples per second).
    // This does *not* change the sample data, just the rate at which
    // the samples would be played back.
    void SetRate(unsigned Hz) { m_rate = Hz; }

    //--------------------------------------------------
    // Information
    //--------------------------------------------------

    // Returns the waveform's sample rate in Hertz.
    unsigned GetRate() const { return m_rate; }

    // Returns the number of interleaved channels in the waveform.
    size_t GetNumChannels() const { return m_numChannels; }

    // Returns the total number of samples in the waveform.
    size_t GetNumSamples() const;

    // Returns the total size of the sample buffer in bytes.
    size_t GetTotalBytes() const;

    // Returns the duration of the waveform in seconds.
    float GetDurationInSeconds() const;

    // Returns the time offset in seconds that corresponds to the
    // specified sample index in the waveform.
    float SampleIndexToTime(size_t index) const;

    // Returns the sample index that corresponds to the specified
    // time offset in the waveform.
    size_t TimeToSampleIndex(float seconds) const;

    // Access the buffer of audio samples.
    const float *GetSamplesPtr() const { return reinterpret_cast<const float *>(m_data.data()); }
    float       *GetSamplesPtr()       { return reinterpret_cast<float *>      (m_data.data()); }

    // Retrieves the sample value at the specified index.
    float GetSample(size_t sampleIndex, size_t channel = 0) const;

    // Scans the samples and returns the highest (most positive)
    // sample value in the waveform.
    float GetHighestSample() const;

    // Scans the samples and returns the lowest (most negative)
    // sample value in the waveform.
    float GetLowestSample() const;

    // Scans the samples of the specified channel of the waveform
    // and returns the index of the sample with the highest (most
    // positive) sample value.
    size_t FindHighestSample(size_t channel = 0) const;

    // Scans the samples of the specified channel of the waveform
    // and returns the index of the sample with the lowest (most
    // negative) sample value.
    size_t FindLowestSample(size_t channel = 0) const;

    //--------------------------------------------------
    // Modify
    //--------------------------------------------------

    // Set the sample value at the specified index.
    bool SetSample(size_t sampleIndex, size_t channel, float value);

    // Fills "count" samples starting at sample number "start"
    // with silence (with zeroes).  If 'soft' is true, each
    // silence begins with the waveform being tapered down to
    // zero over a fraction of a second; otherwise each silence
    // begins with the level abruptly dropping to zero.
    void Silence(size_t start, size_t count, bool soft = true);

    // Deletes "count" samples starting at sample number "start".
    // Returns true if successful.
    bool Delete(size_t start, size_t count);

    // Inserts "count" samples starting at sample number "start".
    // The inserted samples are silent.
    bool Insert(size_t start, size_t count);

    // Stretches or shrinks the waveform to fit in the indicated
    // number of samples.  This alters the perceived pitch.
    // Returns true if successful.
    bool Waveform::Stretch(size_t newNumSamples);

    // Resamples the waveform for playback at the specified sample
    // rate in Hertz.  The total number of samples may changes.
    // Returns true if successful.
    bool Resample(unsigned Hz);

    // Converts a multi-channel waveform to mono by attenuating
    // and mixing all channels into one.
    bool ConvertToMono();

    // Converts a mono waveform to stereo by duplicating the
    // mono signal into both the left and right channels of
    // the converted waveform. 
    bool ConvertToStereo();

    // Multiplies all samples in the waveform by the given value.
    bool Multiply(float value);

    // Adds the given value to all samples in the waveform.
    bool Add(float value);

    // Clips all samples to the given range.
    bool Clip(float lowest = -1.0f, float highest = 1.0f);

    // Changes the volume and offset of the waveform such that
    // it fits within the given range of sample values.
    bool Fit(float lowest = -1.0f, float highest = 1.0f);

    // Normalizes the waveform such that the level doesn't
    // exceed the specified dB level, where 0dB is loudest
    // and -100dB is quietest.
    void Normalize(float dbLevel = -1.0f);

    //--------------------------------------------------
    // Miscellaneous
    //--------------------------------------------------

    // Clips a floating-point value to the given range.
    static float ClipValue(float value, float low = -1.0f, float high = 1.0f)
    {
        return (value < low) ? low : ((value > high) ? high : value);
    }
    static double ClipValue(double value, double low = -1.0f, double high = 1.0f)
    {
        return (value < low) ? low : ((value > high) ? high : value);
    }

private:
    std::vector<float> m_data;  // Buffer of raw PCM audio data.
    unsigned m_rate = 48000;    // Sample rate in Hertz.
    size_t m_numChannels;       // 1=mono, 2=stereo.
};

