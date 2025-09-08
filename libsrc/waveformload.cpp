//-------------------------------------------------------------------
//
// waveformload.cpp
// C++ functions for reading audio files into a Waveform object.
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

#include "waveformload.h"
#include "wavfile.h"
#include "rawpcmfile.h"
#define MINIMP3_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable:4244)
#include "../dependencies/minimp3/minimp3.h"
#pragma warning(pop)

//#define TRACE

// Converts one raw audio sample from a WAV file into our internal
// floating-point format.
static float ConvertWAVSampleToFloat(const WAVInfo &hdr, const void *psample)
{
    if (hdr.m_is_float)
    {
        if (hdr.m_bits == 32)
            return *(reinterpret_cast<const float *>(psample));
 
        if (hdr.m_bits == 64)
            return static_cast<float>(*(reinterpret_cast<const double *>(psample)));
    }
    else
    {
        if (hdr.m_bits == 8)
        {
            uint8_t rawsample = *reinterpret_cast<const uint8_t *>(psample);
            return (static_cast<float>(rawsample) - 128.0f) / 127.0f;
        }
        else if (hdr.m_bits == 16)
        {
            int16_t rawsample = *reinterpret_cast<const int16_t *>(psample);
            return static_cast<float>(rawsample) / static_cast<float>(0x7FFF);
        }
        else if (hdr.m_bits == 32)
        {
            int32_t rawsample = *reinterpret_cast<const int32_t *>(psample);
            return static_cast<float>(rawsample) / static_cast<float>(0x7FFFFFFF);
        }
    }

    // Unsupported format.
    return 0.0f;
}

// Converts one audio sample from a raw PCM file into our internal
// floating-point format.
static float ConvertRawSampleToFloat(bool isFloat, unsigned bytesPerSample, const void *psample)
{
    if (isFloat)
    {
        if (bytesPerSample == 4)
            return *(reinterpret_cast<const float *>(psample));
 
        if (bytesPerSample == 8)
            return static_cast<float>(*(reinterpret_cast<const double *>(psample)));
    }
    else
    {
        if (bytesPerSample == 1)
        {
            uint8_t rawsample = *reinterpret_cast<const uint8_t *>(psample);
            return (static_cast<float>(rawsample) - 128.0f) / 127.0f;
        }
        else if (bytesPerSample == 2)
        {
            int16_t rawsample = *reinterpret_cast<const int16_t *>(psample);
            return static_cast<float>(rawsample) / static_cast<float>(0x7FFF);
        }
        else if (bytesPerSample == 4)
        {
            int32_t rawsample = *reinterpret_cast<const int32_t *>(psample);
            return static_cast<float>(rawsample) / static_cast<float>(0x7FFFFFFF);
        }
    }

    // Unsupported format.
    return 0.0f;
}

//
// Loads the audio data from a Microsoft WAV audio file, placing
// the audio data into the given Waveform object.  Returns true
// if successful, false if error.
//
static bool WaveformLoadFromWAV(
        const wchar_t *filename,
        Waveform &wav,
        void *status_callback_context,
        bool (*status_callback_func)(void *context, float completion)
        )
{
#ifdef TRACE
    printf("WaveformLoadFromWAV '%S'\n", filename);
    fflush(stdout);
#endif

    if (status_callback_func && !status_callback_func(status_callback_context, 0.0f))
        return false;

    // Determine the format of the WAV data.
    WAVInfo hdr;
    if (!WAVFileReadHeader(filename, hdr))
        return false;

#ifdef TRACE
    printf("  rate=%u channels=%u bits=%u isfloat=%c sample_count=%u\n",
        hdr.m_rate, hdr.m_channels, hdr.m_bits,
        hdr.m_is_float ? 'Y' : 'N', hdr.m_sample_count);
#endif

    wav.SetRate(hdr.m_rate);

    if (status_callback_func && !status_callback_func(status_callback_context, 0.1f))
        return false;

    // Read the raw PCM data.
    std::vector<uint8_t> data(hdr.m_channels * hdr.m_sample_count * hdr.m_bits / 8);
    if (!WAVFileReadSamples(filename, data.data(), data.size()))
        return false;

    if (status_callback_func && !status_callback_func(status_callback_context, 0.3f))
        return false;

    // Allocate space for the converted PCM data.
    if (!wav.Populate(hdr.m_sample_count, hdr.m_channels))
        return false;

    if (status_callback_func && !status_callback_func(status_callback_context, 0.4f))
        return false;

    // Convert the data to our internal floating-point format.
    // TODO:  Call the status update function occasionally during this.
    const uint8_t *pinsample = data.data();
    float *poutsample = wav.GetSamplesPtr();
    for (unsigned isample = 0; isample < hdr.m_sample_count; isample++)
    {
        for (unsigned i = 0; i < hdr.m_channels; i++)
        {
            *poutsample++ = ConvertWAVSampleToFloat(hdr, pinsample);
            pinsample += hdr.m_bits / 8;
        }
    }

    if (status_callback_func && !status_callback_func(status_callback_context, 1.0f))
    {
        wav = Waveform();
        return false;
    }

    return true;
}

//
// Loads the audio data from a raw PCM audio file, placing
// the audio data into the given Waveform object.  Returns true
// if successful, false if error.
//
static bool WaveformLoadFromRawPCM(
        const wchar_t *filename,
        Waveform &wav,
        unsigned rate,
        unsigned bytesPerSample,
        unsigned numChannels,
        bool isFloat,
        void *status_callback_context,
        bool (*status_callback_func)(void *context, float completion)
        )
{
#ifdef TRACE
    printf("WaveformLoadFromRawPCM '%S'\n", filename);
    printf("  rate=%u  bytesPerSample=%u  numChannels=%u  isFloat=%c\n",
        rate, bytesPerSample, numChannels, isFloat ? 'Y' : 'N');
    fflush(stdout);
#endif

    if (status_callback_func && !status_callback_func(status_callback_context, 0.0f))
        return false;

    // Determine the number of samples based on the requested
    // sample format and the size of the file.
    uint64_t fileBytes = RawPCMFileGetSizeInBytes(filename);
    if (fileBytes < 1)
        return false;
    size_t numSamples = fileBytes / (numChannels * bytesPerSample);
    if (numSamples < 1)
        return false;

    if (status_callback_func && !status_callback_func(status_callback_context, 0.1f))
        return false;

    // Read the raw PCM data.
    std::vector<uint8_t> data(numChannels * numSamples * bytesPerSample);
    if (!RawPCMFileRead(filename, numSamples, numChannels, bytesPerSample, data.data(), data.size()))
        return false;

    if (status_callback_func && !status_callback_func(status_callback_context, 0.3f))
        return false;

    // Allocate space for the converted PCM data.
    if (!wav.Populate(numSamples, numChannels))
        return false;

    if (status_callback_func && !status_callback_func(status_callback_context, 0.4f))
        return false;

    // Convert the data to our internal floating-point format.
    // TODO:  Call the status update function occasionally during this.
    const uint8_t *pinsample = data.data();
    float *poutsample = wav.GetSamplesPtr();
    for (size_t isample = 0; isample < numSamples; isample++)
    {
        for (unsigned channel = 0; channel < numChannels; channel++)
        {
            *poutsample++ = ConvertRawSampleToFloat(isFloat, bytesPerSample, pinsample);
            pinsample += bytesPerSample;
        }
    }

    if (status_callback_func && !status_callback_func(status_callback_context, 1.0f))
    {
        wav = Waveform();
        return false;
    }

    wav.SetRate(rate);

    return true;
}

//
// Reads the raw data from the given file, and returns
// it as a vector of bytes.
//
static std::vector<uint8_t> ReadFile(const wchar_t *filename)
{
#ifdef TRACE
    printf("ReadFile '%S'\n", filename);
#endif

    std::vector<uint8_t> data;

    FILE *fp = nullptr;
    if (_wfopen_s(&fp, filename, L"rb") || !fp)
        return data;

    if (_fseeki64(fp, 0, SEEK_END))
    {
        fclose(fp);
        return data;
    }

    int64_t file_size = _ftelli64_nolock(fp);
#ifdef TRACE
    printf("  File size = %I64d\n", file_size);
#endif
    if (file_size < 1)
    {
        fclose(fp);
        return data;
    }

    data.resize(static_cast<size_t>(file_size));
    _fseeki64(fp, 0, SEEK_SET);
    if (fread(data.data(), 1, data.size(), fp) != data.size())
    {
        fclose(fp);
        data.clear();
        return data;
    }

    fclose(fp);
    return data;
}

//
// Loads the audio data from an MP3 audio file, placing the
// audio data into the given Waveform object.  Returns true
// if successful, false if error.
//
static bool WaveformLoadFromMP3(
        const wchar_t *filename,
        Waveform &wav,
        void *status_callback_context,
        bool (*status_callback_func)(void *context, float completion)
        )
{
#ifdef TRACE
    printf("WaveformLoadFromMP3 '%S'\n", filename);
    fflush(stdout);
#endif

    if (status_callback_func && !status_callback_func(status_callback_context, 0.0f))
        return false;

    // Load the bytes from the MP3 into memory.
    std::vector<uint8_t> filedata = ReadFile(filename);
    if (filedata.empty())
        return false;

#ifdef TRACE
    printf("  ReadFile got %zu bytes.\n", filedata.size());
    fflush(stdout);
#endif

    // Initialize decoder object.
    mp3dec_t mp3d = {0};
    mp3dec_init(&mp3d);

    // These will be filled in with the format info found in
    // the MP3 frames.
    int rate_found = 0;
    int channels_found = 0;

#ifdef TRACE
    printf("Decoding MP3 frames.\n");
    fflush(stdout);
#endif

    // Decode all frames from the MP3 data into raw PCM data.
    std::vector<uint8_t> pcmdata;
    mp3dec_frame_info_t info;
    int16_t pcm_frame[MINIMP3_MAX_SAMPLES_PER_FRAME] = {0};
    int frame_samples = 0;
    int frame_offset = 0;
    while ((frame_samples = mp3dec_decode_frame(&mp3d,
                                &filedata[frame_offset], (int)filedata.size() - frame_offset,
                                pcm_frame, &info)) > 0)
    {
        rate_found = info.hz;
        channels_found = info.channels;

        // Append the new samples to the end of the pcmdata buffer.
        size_t offset = pcmdata.size();
        size_t newBytes = frame_samples * sizeof(int16_t) * info.channels;
        pcmdata.resize(pcmdata.size() + newBytes);
        memcpy(&pcmdata[offset], pcm_frame, newBytes);

        frame_offset += info.frame_bytes;
    }

#ifdef TRACE
    printf("  rate_found=%d channels_found=%d pcmdata.size=%zu\n", rate_found, channels_found, pcmdata.size());
    fflush(stdout);
#endif

    // Convert the PCM file data we just read into our internal format.
    size_t numNewSamples = pcmdata.size() / sizeof(int16_t) / channels_found;
    wav.SetRate(rate_found);
    wav.Populate(numNewSamples, channels_found);
    const int16_t *pinsample = reinterpret_cast<int16_t *>(pcmdata.data());
    float *poutsample = wav.GetSamplesPtr();
    for (size_t isample = 0; isample < numNewSamples; isample++)
    {
        for (size_t ichannel = 0; ichannel < channels_found; ichannel++)
        {
            float sample = static_cast<float>(*pinsample++) / static_cast<float>(0x7FFF);
            *poutsample++ = sample;
        }
    }

    if (status_callback_func && !status_callback_func(status_callback_context, 1.0f))
    {
        wav = Waveform();
        return false;
    }

    return true;
}

//
// Loads the specified audio file, placing the audio data
// into the given Waveform object.  Returns true if
// successful, false if error.
//
// If a pointer to status callback function is provided,
// it will be called periodically during the loading
// procedure, with a completion parameter value ranging
// from 0.0 to 1.0 to indicate the relative completeness
// of the loading operation.  If the status function
// returns false, the loading is immediately aborted.
// The status callback mechanism is provided so that the
// caller may update a status display if desired.
//
bool WaveformLoadFromFile(
        const wchar_t *filename,
        Waveform &wav,
        void *status_callback_context,
        bool (*status_callback_func)(void *context, float completion)
        )
{
#ifdef TRACE
    printf("WaveformLoadFromFile '%S'\n", filename);
    fflush(stdout);
#endif

    const wchar_t *extension = wcsrchr(filename, '.');
    if (_wcsicmp(extension, L".wav") == 0)
    {
        return WaveformLoadFromWAV(filename, wav, status_callback_context, status_callback_func);
    }
    else if (_wcsicmp(extension, L".mp3") == 0)
    {
        return WaveformLoadFromMP3(filename, wav, status_callback_context, status_callback_func);
    }
    else if (_wcsicmp(extension, L".raw") == 0 ||
             _wcsicmp(extension, L".pcm") == 0)
    {
        // TODO: Need a way to specify the sample format externally.
        unsigned rate = 22500;
        unsigned bytesPerSample = 2;
        unsigned numChannels = 2;
        bool isFloat = false;

        return WaveformLoadFromRawPCM(filename, wav,
                    rate, bytesPerSample, numChannels, isFloat,
                    status_callback_context, status_callback_func);
    }

    // Unrecognized filename extension!
#ifdef TRACE
    printf("Unrecognized filename extension on '%S'\n", filename);
#endif
    return false;
}

