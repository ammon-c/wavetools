//-------------------------------------------------------------------
//
// waveform.cpp
// C++ utility container class for a PCM audio waveform.
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

//--------------------------------------------------
// Initialize
//--------------------------------------------------

// Populates this object with an audio signal of the specified
// duration and number of channels.  If "data" is non-null, the
// contents of "data" are copied into the Waveform's audio
// sample buffer in memory; otherwise the audio signal is
// initialized to silence (all zeroes).
// Returns true if successful.
bool Waveform::Populate(
        size_t numSamples,
        size_t numChannels,
        const float *sampleData)
{
    // Check for bad/unsupported formats.
    if (numChannels < 1 || numChannels > 256)
        return false;

    m_numChannels = numChannels;
    m_data.resize(numSamples * numChannels);
    size_t numBytes = numSamples * numChannels * sizeof(float);
    if (numBytes > 0)
    {
        if (sampleData != nullptr)
            memcpy(m_data.data(), sampleData, numBytes);
        else
            memset(m_data.data(), 0, numBytes);
    }

    return true;
}

//--------------------------------------------------
// Information
//--------------------------------------------------

// Returns the total number of samples in the waveform.
size_t Waveform::GetNumSamples() const
{
    if (m_numChannels == 0 || m_data.empty())
        return 0;

    return m_data.size() / m_numChannels;
}

// Returns the total size of the sample buffer in bytes.
size_t Waveform::GetTotalBytes() const
{
    return m_data.size();
}

// Returns the duration of the waveform in seconds.
float Waveform::GetDurationInSeconds() const
{
    if (m_rate == 0 || m_data.empty())
        return 0.0f;

    return GetNumSamples() / static_cast<float>(m_rate);
}

// Returns the time offset in seconds that corresponds to the
// specified sample index in the waveform.
float Waveform::SampleIndexToTime(size_t index) const
{
    size_t numSamples = GetNumSamples();
    if (numSamples == 0)
        return 0.0f;

    return (static_cast<float>(index) / numSamples) * GetDurationInSeconds();
}

// Returns the sample index that corresponds to the specified
// time offset in the waveform.
size_t Waveform::TimeToSampleIndex(float seconds) const
{
    if (seconds <= 0.0 || m_data.empty())
        return 0;

    size_t numSamples = GetNumSamples();
    if (numSamples == 0)
        return 0;

    return static_cast<size_t>(seconds / GetDurationInSeconds() * numSamples);
}

// Retrieves the sample value at the specified index.
float Waveform::GetSample(size_t sampleIndex, size_t channel) const
{
    if (sampleIndex > GetNumSamples() || channel >= m_numChannels || m_data.empty())
        return 0.0f;

    return m_data[sampleIndex * m_numChannels + channel];
}

// Scans the samples and returns the highest (most positive)
// sample value in the waveform.
float Waveform::GetHighestSample() const
{
    if (m_numChannels < 1 || m_data.empty())
        return 0.0f;

    float highest = -FLT_MAX;
    size_t count = m_data.size();
    for (size_t index = 0; index < count; index++)
    {
        if (m_data[index] > highest)
            highest = m_data[index];
    }

    return highest;
}

// Scans the samples and returns the lowest (most negative)
// sample value in the waveform.
float Waveform::GetLowestSample() const
{
    if (m_numChannels < 1 || m_data.empty())
        return 0.0f;

    float lowest = FLT_MAX;
    size_t count = m_data.size();
    for (size_t index = 0; index < count; index++)
    {
        if (m_data[index] < lowest)
            lowest = m_data[index];
    }

    return lowest;
}

// Scans the samples of the specified channel of the waveform
// and returns the index of the sample with the highest (most
// positive) sample value.
size_t Waveform::FindHighestSample(size_t channel) const
{
    if (m_data.empty() || channel >= m_numChannels)
        return 0;

    float high = -FLT_MAX;
    size_t highIndex = 0;
    size_t numSamples = GetNumSamples();
    for (size_t index = 0; index < numSamples; index++)
    {
        float sample = m_data[index * m_numChannels + channel];
        if (sample > high)
        {
            high = sample;
            highIndex = index;
        }
    }

    return highIndex;
}

// Scans the samples of the specified channel of the waveform
// and returns the index of the sample with the lowest (most
// negative) sample value.
size_t Waveform::FindLowestSample(size_t channel) const
{
    if (m_data.empty() || channel >= m_numChannels)
        return 0;

    float low = FLT_MAX;
    size_t lowIndex = 0;
    size_t numSamples = GetNumSamples();
    for (size_t index = 0; index < numSamples; index++)
    {
        float sample = m_data[index * m_numChannels + channel];
        if (sample < low)
        {
            low = sample;
            lowIndex = index;
        }
    }

    return lowIndex;
}

//--------------------------------------------------
// Modify
//--------------------------------------------------

// Set the sample value at the specified index.
bool Waveform::SetSample(size_t sampleIndex, size_t channel, float value)
{
    size_t numSamples = GetNumSamples();
    if (sampleIndex >= numSamples || channel >= m_numChannels)
        return false;

    m_data[sampleIndex * m_numChannels + channel] = value;
    return true;
}

bool Waveform::ConvertToMono()
{
    if (m_data.empty())
    {
        m_numChannels = 1;
        return true; // No data to convert.
    }

    if (m_numChannels == 1)
        return true; // Already in the requested format.

    if (m_numChannels < 1)
        return false; // Invalid channel count!

    size_t numSamples = GetNumSamples();
    std::vector<float> newData(numSamples);

    for (size_t index = 0; index < numSamples; index++)
    {
        float val = 0.0;

        for (size_t channel = 0; channel < m_numChannels; channel++)
            val += GetSample(index, channel) / m_numChannels;

        newData[index] = val;
    }

    m_data = newData;
    m_numChannels = 1;
    return true;
}

bool Waveform::ConvertToStereo()
{
    if (m_data.empty())
    {
        m_numChannels = 2;
        return true; // No data to convert.
    }

    if (m_numChannels < 1 || m_numChannels > 2)
        return false; // Invalid channel count!

    if (m_numChannels == 2)
        return true; // Already in the requested format.

    size_t numSamples = GetNumSamples();
    std::vector<float> newData(numSamples * 2);

    for (size_t index = 0; index < numSamples; index++)
    {
        float val = GetSample(index, 0);

        for (size_t channel = 0; channel < 2; channel++)
            newData[index * 2 + channel] = val;
    }

    m_data = newData;
    m_numChannels = 2;
    return true;
}

// Fills "count" samples starting at sample number "start"
// with silence (with zeroes).  If 'soft' is true, each
// silence begins with the waveform being tapered down to
// zero over a fraction of a second; otherwise each silence
// begins with the level abruptly dropping to zero.
void Waveform::Silence(size_t start, size_t count, bool soft)
{
    if (m_data.empty() || m_numChannels < 1 || count < 1)
        return;

    size_t numSamples = m_data.size() / m_numChannels;
    if (start >= numSamples)
        return;

    if (count + start > numSamples)
        count = numSamples - start;

    size_t end = count * m_numChannels;
    float *sample = m_data.data() + start * m_numChannels;
    const size_t numSamplesToSoftSilence = m_rate * m_numChannels / 10;
    for (size_t index = 0; index < end; index++)
    {
        if (soft && index < numSamplesToSoftSilence)
        {
            // For the first few samples we ramp down the volume.
            float value = *sample;
            value = value * static_cast<float>(numSamplesToSoftSilence - 1 - index) / numSamplesToSoftSilence;
            *sample++ = value;
        }
#if 0
// TODO: This needs more work.
        else if (soft && index > (end - numSamplesToSoftSilence))
        {
            // For the last few samples we ramp up the volume.
            float value = *sample;
            value = value * static_cast<float>(numSamplesToSoftSilence - 1 - (end - index)) / numSamplesToSoftSilence;
            *sample++ = value;
        }
#endif
        else
        {
            *sample++ = 0.0f;
        }
    }
}

// Deletes "count" samples starting at sample number "start".
// Returns true if successful.
bool Waveform::Delete(size_t start, size_t count)
{
    if (m_data.empty())
        return true;

    size_t numSamples = GetNumSamples();
    if (start >= numSamples)
        return false; // Out of range!
    if (start + count >= numSamples)
        count = numSamples - start;

    m_data.erase(m_data.begin() + (start * m_numChannels),
                 m_data.begin() + ((start + count) * m_numChannels));

    return true;
}

// Inserts "count" samples starting at sample number "start".
// The inserted samples are silent.
bool Waveform::Insert(size_t start, size_t count)
{
    if (m_data.empty())
    {
        if (start == 0)
            return Populate(count, m_numChannels);

        return false;
    }

    size_t numSamples = GetNumSamples();
    if (start > numSamples)
        return false; // Out of range!

    m_data.insert(m_data.begin() + (start * m_numChannels), count * m_numChannels, 0.0f);
    return true;
}

// Stretches or shrinks the waveform to fit in the indicated
// number of samples.  This alters the perceived pitch.
// Returns true if successful.
bool Waveform::Stretch(size_t newNumSamples)
{
    if (newNumSamples < 1)
        return false;

    if (m_data.size() <= 1)
        return true;

    size_t numSamples = GetNumSamples();
    Waveform newWav;
    newWav.SetRate(m_rate);
    newWav.Populate(newNumSamples, m_numChannels);

    for (size_t newIndex = 0; newIndex < newNumSamples; newIndex++)
    {
        // TODO: Rather than just getting the "nearest neighbor"
        // sample, it would be more accurate and sound better
        // if we interpolated between the 2 nearest samples.

        size_t oldIndex = static_cast<size_t>(newIndex * static_cast<double>(numSamples) / newNumSamples);
        if (oldIndex >= numSamples)
            continue;

        memcpy(&newWav.m_data[newIndex * m_numChannels],
               &m_data[oldIndex * m_numChannels],
               sizeof(float) * m_numChannels);
    }

    // Replace ourselves with the resampled waveform.
    *this = newWav;

    return true;
}

// Resamples the waveform for playback at the specified sample
// rate in Hertz.  The total number of samples may changes.
// Returns true if successful.
bool Waveform::Resample(unsigned Hz)
{
    if (Hz < 1)
        return false;

    if (m_data.size() <= 1)
    {
        m_rate = Hz;
        return true;
    }

    // Calculate the size of the resampled waveform.
    size_t numSamples = GetNumSamples();
    size_t newNumSamples = static_cast<size_t>(numSamples * static_cast<double>(Hz) / m_rate);
    if (newNumSamples < 1)
        newNumSamples = 1;

    // Stretch the waveform to the calculated size.
    if (!Stretch(newNumSamples))
    {
        return false;
    }

    m_rate = Hz;

    return true;
}

// Multiplies all samples in the waveform by the given value.
bool Waveform::Multiply(float value)
{
    if (m_data.empty())
        return false;

    size_t numSamples = GetNumSamples();
    for (size_t index = 0; index < numSamples; index++)
    {
        for (size_t channel = 0; channel < m_numChannels; channel++)
        {
            m_data[index * m_numChannels + channel] *= value;
        }
    }

    return true;
}

// Adds the given value to all samples in the waveform.
bool Waveform::Add(float value)
{
    if (m_data.empty())
        return false;

    size_t numSamples = GetNumSamples();
    for (size_t index = 0; index < numSamples; index++)
    {
        for (size_t channel = 0; channel < m_numChannels; channel++)
        {
            m_data[index * m_numChannels + channel] += value;
        }
    }

    return true;
}

// Clips all samples to the given range.
bool Waveform::Clip(float lowest, float highest)
{
    if (m_data.empty())
        return true;

    size_t numSamples = GetNumSamples();
    for (size_t index = 0; index < numSamples; index++)
    {
        for (size_t channel = 0; channel < m_numChannels; channel++)
        {
            float sample = m_data[index * m_numChannels + channel];

            if (sample < lowest)
                sample = lowest;
            if (sample > highest)
                sample = highest;

            m_data[index * m_numChannels + channel] = sample;
        }
    }

    return true;
}

// Changes the volume and offset of the waveform such that
// it fits within the given range of sample values.
bool Waveform::Fit(float lowest, float highest)
{
    if (m_data.empty())
        return true;
    if (lowest >= highest)
        return false;

    size_t indexLowest = FindLowestSample();
    size_t indexHighest = FindHighestSample();
    float dataLowest = GetSample(indexLowest, 0);
    float dataHighest = GetSample(indexHighest, 0);

    float delta = highest - lowest;
    float dataDelta = dataHighest - dataLowest;
    const float tiny_value = 1.0e-6f;
    if (delta < tiny_value || dataDelta < tiny_value)
        return false;

    size_t numSamples = GetNumSamples();
    for (size_t index = 0; index < numSamples; index++)
    {
        for (size_t channel = 0; channel < m_numChannels; channel++)
        {
            float sample = m_data[index * m_numChannels + channel];

            sample -= dataLowest;
            sample = sample * delta / dataDelta;
            sample += lowest;

            m_data[index * m_numChannels + channel] = sample;
        }
    }

    return true;
}

// From an attenuation level between 0 dB (loudest) and -infinity
// dB (quietest), returns the corresponding linear gain multiplier
// value.
static float dbToLinear(float db)
{
    return powf(10.0f, db / 20.0f);
}

// Normalizes the waveform such that the level doesn't
// exceed the specified dB level, where 0dB is loudest
// and -100dB is quietest.
void Waveform::Normalize(float dbLevel)
{
    if (dbLevel > 0.0f)
        dbLevel = 0.0f;
    if (dbLevel < -100.0f)
        dbLevel = -100.0f;

    if (m_data.empty() || m_numChannels < 1 || m_rate < 1)
        return;

    //
    // Examine the waveform in chunks of about 10 milliseconds
    // each, finding the peak volume level of each chunk as we go.
    //
    // If the current chunk's audio peak is lower than the target
    // maximum, we increase the gain slightly.  If the peak is
    // higher than the target maximum, reduce the gain abruptly.
    //
    // So it behaves kind of like an analog microphone compressor.
    //

    const float maxVol = dbToLinear(dbLevel);
    const unsigned samplesPerChunk = static_cast<unsigned>(m_rate * 0.01f * m_numChannels);
    const unsigned numChunks = static_cast<unsigned>(m_data.size() / samplesPerChunk);
    float gain = 1.0f;

    for (unsigned chunk = 0; chunk < numChunks; chunk++)
    {
        // Determine the peak volume of the samples in this chunk.
        unsigned isample = static_cast<unsigned>(chunk * samplesPerChunk);
        float local_peak = 0.0f;
        for (unsigned subsample = 0; subsample < samplesPerChunk; subsample++)
        {
            float vol = fabsf(m_data[isample + subsample]);
            if (vol > local_peak)
                local_peak = vol;
        }

        // If this chunks's peak volume is less than the target max,
        // gradually increase the gain.
        if (local_peak < maxVol && gain < 100.0f)
            gain *= 1.05f;

        // If this chunks's peak volume exceeds the target max,
        // drop the gain abruptly. 
        if (local_peak * gain > maxVol)
        {
            if (local_peak < 0.02f)
                gain = maxVol / 0.02f;
            else
                gain = maxVol / local_peak;
        }

        // Apply the gain multiplier to the samples in this chunk.
        // If this is the last full chunk, also apply the gain to
        // any remaining partial chunk at the very end of the
        // waveform.
        size_t count = samplesPerChunk;
        if (chunk == numChunks - 1)
            count = m_data.size() - isample;
        for (size_t subsample = 0; subsample < count; subsample++)
            m_data[isample + subsample] *= gain;
    }
}

