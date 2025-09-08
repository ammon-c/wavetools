//-------------------------------------------------------------------
//
// waveformsave.cpp
// C++ functions for writing a Waveform object to an audio file.
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

#include "waveformsave.h"
#include "wavfile.h"
#include "rawpcmfile.h"
#include <string>
#include <process.h>
#include <io.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//#define TRACE

//
// Converts a sample value from our internal floating-point
// format to one of the supported output formats.
//
static bool ConvertFloatSample(const float *insample, void *outsample,
    bool isFloat, unsigned bytesPerSample)
{
    if (isFloat)
    {
        if (bytesPerSample == 4)
        {
            *reinterpret_cast<float *>(outsample) = *insample;
            return true;
        }
        else if (bytesPerSample == 8)
        {
            *reinterpret_cast<double *>(outsample) = static_cast<double>(*insample);
            return true;
        }
    }
    else
    {
        if (bytesPerSample == 1)
        {
            float sample = (*insample) * 127.0f + 128.0f;
            if (sample < 0.0f)
                sample = 0.0f;
            if (sample > 255.0)
                sample = 255.0f;
            *reinterpret_cast<uint8_t *>(outsample) = static_cast<uint8_t>(sample);
            return true;
        }
        else if (bytesPerSample == 2)
        {
            float sample = (*insample) * static_cast<float>(0x7FFF);
            if (sample < static_cast<float>(-0x7FFF))
                sample = static_cast<float>(-0x7FFF);
            if (sample > static_cast<float>(0x7FFF))
                sample = static_cast<float>(0x7FFF);
            *reinterpret_cast<uint16_t *>(outsample) = static_cast<uint16_t>(sample);
            return true;
        }
        else if (bytesPerSample == 4)
        {
            float sample = (*insample) * static_cast<float>(0x7FFFFFFF);
            if (sample < static_cast<float>(-0x7FFFFFFF))
                sample = static_cast<float>(-0x7FFFFFFF);
            if (sample > static_cast<float>(0x7FFFFFFF))
                sample = static_cast<float>(0x7FFFFFFF);
            *reinterpret_cast<uint32_t *>(outsample) = static_cast<uint32_t>(sample);
            return true;
        }
    }

    // Unsupported format.
    return false;
}

//
// Saves the Waveform's audio data to a Microsoft WAV audio file.
// Returns true if successful, false if error.
//
static bool WaveformSaveToWAV(
        const wchar_t *filename,
        const Waveform &wav,
        void *status_callback_context,
        bool (*status_callback_func)(void *context, float completion),
        bool useFloat,
        unsigned useBytesPerSample
        )
{
#ifdef TRACE
    printf("WaveformSaveToWAV '%S', %u Hz\n", filename, wav.GetRate());
    fflush(stdout);
#endif

    if (status_callback_func && !status_callback_func(status_callback_context, 0.0f))
        return false;

    if (useFloat && useBytesPerSample == 8)
    {
        // WAV doesn't support doubles, so we'll write floats instead.
        useBytesPerSample = 4;
    }

    // Convert the internal floating-point data to the data
    // format the caller requested for the saved file.
    size_t numChannels = wav.GetNumChannels();
    size_t numSamples = wav.GetNumSamples();
    std::vector<uint8_t> data(numSamples * wav.GetNumChannels() * useBytesPerSample);
    const float *insample = wav.GetSamplesPtr();
    uint8_t *outsample = data.data();
    for (size_t isample = 0; isample < numSamples; isample++)
    {
        for (size_t channel = 0; channel < numChannels; channel++)
        {
            if (!ConvertFloatSample(insample, outsample,
                                    useFloat, useBytesPerSample))
            {
                return false;
            }

            insample++;
            outsample += useBytesPerSample;
        }
    }

    // Write the converted data to WAV file.
    WAVInfo info;
    info.m_rate = wav.GetRate();
    info.m_channels = static_cast<unsigned>(numChannels);
    info.m_bits = useBytesPerSample * 8;
    info.m_is_float = useFloat;
    info.m_sample_count = static_cast<unsigned>(wav.GetNumSamples());
    if (!WAVFileWrite(filename, info, data.data()))
    {
        return false;
    }

    if (status_callback_func && !status_callback_func(status_callback_context, 1.0f))
        return false;

    return true;
}

//
// Saves the Waveform's audio data to a raw PCM audio file.
// The raw PCM file will contain floating-point data unless
// 'saveAsInt16' is true, in which case the data will be
// written as 16-bit integers.
// Returns true if successful, false if error.
//
static bool WaveformSaveToRawPCM(
        const wchar_t *filename,
        const Waveform &wav,
        void *status_callback_context,
        bool (*status_callback_func)(void *context, float completion),
        bool useFloat,
        unsigned useBytesPerSample
        )
{
#ifdef TRACE
    printf("WaveformSaveToRawPCM '%S'\n", filename);
    fflush(stdout);
#endif

    if (status_callback_func && !status_callback_func(status_callback_context, 0.0f))
        return false;

    // Convert the internal floating-point data to the data
    // format the caller requested for the saved file.
    size_t numChannels = wav.GetNumChannels();
    size_t numSamples = wav.GetNumSamples();
    std::vector<uint8_t> data(numSamples * numChannels * useBytesPerSample);
    const float *insample = wav.GetSamplesPtr();
    uint8_t *outsample = data.data();
    for (size_t isample = 0; isample < numSamples; isample++)
    {
        for (size_t channel = 0; channel < numChannels; channel++)
        {
            if (!ConvertFloatSample(insample, outsample,
                                    useFloat, useBytesPerSample))
            {
#ifdef TRACE
                printf("ConvertFloatSample failed.\n");
#endif
                return false;
            }

            insample++;
            outsample += useBytesPerSample;
        }
    }

    if (!RawPCMFileWrite(filename, wav.GetNumSamples(),
            static_cast<unsigned>(numChannels), useBytesPerSample,
            data.data()))
    {
#ifdef TRACE
        printf("RawPCMFileWrite failed.\n");
#endif
        return false;
    }

    if (status_callback_func && !status_callback_func(status_callback_context, 1.0f))
        return false;

    return true;
}

// Checks if the ffmpeg.exe program exists in the directory
// where our .exe lives.  Returns the full path if successful.
// Returns an empty string otherwise.
static std::wstring GetFfmpegPath()
{
    wchar_t exePath[MAX_PATH] = {0};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    wchar_t *slash = wcsrchr(exePath, '\\');
    if (!slash)
        slash = &exePath[0];
    else
        slash++;
    *slash = '\0';

    wcscat_s(exePath, MAX_PATH, L"ffmpeg.exe");

    // Check if the file exists.
    DWORD attrib = GetFileAttributesW(exePath);
    if (attrib == INVALID_FILE_ATTRIBUTES)
        return L"";
    if (attrib & FILE_ATTRIBUTE_DIRECTORY)
        return L"";

    return exePath;
}

//
// Saves the Waveform's audio data to an MP3 audio file.
// Returns true if successful, false if error.
//
static bool WaveformSaveToMP3(
        const wchar_t *filename,
        const Waveform &wav,
        void *status_callback_context,
        bool (*status_callback_func)(void *context, float completion)
        )
{
#ifdef TRACE
    printf("WaveformSaveToMP3 '%S'\n", filename);
    fflush(stdout);
#endif

    if (status_callback_func && !status_callback_func(status_callback_context, 0.0f))
        return false;

    //
    // We can't save directly to MP3 format.
    // We have to save to WAV format and then use an external
    // program (ffmpeg) to convert the WAV file to an MP3 file.
    //

    // See if ffmpeg is here.
    std::wstring exePath = GetFfmpegPath();
    if (exePath.empty())
    {
#ifdef TRACE
        printf("Ffmpeg.exe was not found with wavetool binaries.\n");
        printf("Unable to save to MP3 file format!\n");
#endif
        return false;
    }

    // Save the waveform to a temporary WAV file.
    std::wstring tmpFilename = filename;
    tmpFilename += L".wav";
    if (!WaveformSaveToWAV(tmpFilename.c_str(), wav,
            status_callback_context, status_callback_func,
            false, 2))
    {
#ifdef TRACE
        printf("Failed saving temporary file '%S'\n", tmpFilename.c_str());
        fflush(stdout);
#endif
        return false;
    }

    if (status_callback_func && !status_callback_func(status_callback_context, 0.3f))
        return false;

    // Build an argument list for running ffmpeg to convert the
    // temporary WAV file to an MP3 file.
    std::vector<const wchar_t *> args;
    const wchar_t *arg_dash_i = L"-i";
    const wchar_t *arg_acodec = L"-acodec";
    const wchar_t *arg_mp3    = L"mp3";
    args.push_back(exePath.c_str());
    args.push_back(arg_dash_i);
    args.push_back(tmpFilename.c_str());
    args.push_back(arg_acodec);
    args.push_back(arg_mp3);
    args.push_back(filename);
    args.push_back(nullptr);

	// Temporarily redirect stdout and stderr, so ffmpeg's console
    // output doesn't spew all over the terminal.
	FILE *tmp_stream1 = nullptr;
	FILE *tmp_stream2 = nullptr;
	int stream1 = _dup(_fileno(stdout));
	_wfreopen_s(&tmp_stream1, L"NUL:", L"a+", stdout);
	int stream2 = _dup(_fileno(stderr));
	_wfreopen_s(&tmp_stream2, L"NUL:", L"a+", stderr);

    // Run the external ffmpeg program.
    intptr_t result = _wspawnvp(_P_WAIT, args[0], &args[0]);
#ifdef TRACE
    int temp_errno = errno;
#endif

	// Restore the normal behavior of stdout and stderr.
	_dup2(stream2, _fileno(stderr));
	_close(stream2);
	_dup2(stream1, _fileno(stdout));
	_close(stream1);

    _wunlink(tmpFilename.c_str());

    if (result == -1)
    {
        // Failed to launch ffmpeg.
#ifdef TRACE
        printf("Failed spawning ffmpeg.  [errno:%d]\n", temp_errno);
#endif
        return false;
    }
    else if (result != 0)
    {
        // Ffmpeg reported an error.
#ifdef TRACE
        printf("External ffmpeg process reported error.  [exit:%Id]\n", result);
#endif
        return false;
    }

    if (status_callback_func && !status_callback_func(status_callback_context, 1.0f))
        return false;

    return true;
}

//
// Writes the data from a Waveform object to an audio file.
//
// Returns true if successful, false if error.
//
// If a pointer to status callback function is provided,
// it will be called periodically during the saving
// procedure, with a completion parameter value ranging
// from 0.0 to 1.0 to indicate the relative completeness
// of the saving operation.  If the status function
// returns false, the saving is immediately aborted.
// The status callback mechanism is provided so that the
// caller may update a status display if desired.
//
bool WaveformSaveToFile(
        const wchar_t *filename,
        const Waveform &wav,
        void *status_callback_context,
        bool (*status_callback_func)(void *context, float completion),

        // For audio file formats that support both integer and
        // floating-point data, this indicates which to write.
        bool useFloat,

        // For audio file formats that support more than one
        // possible size for audio samples, this indicates which
        // size to write.
        unsigned useBytesPerSample
        )
{
#ifdef TRACE
    printf("WaveformSaveToFile '%S', useFloat=%c, useBytesPerSample=%u\n",
        filename, useFloat ? 'Y' : 'N', useBytesPerSample);
    printf("  Rate=%u\n", wav.GetRate());
    fflush(stdout);
#endif

    const wchar_t *extension = wcsrchr(filename, '.');
    if (_wcsicmp(extension, L".wav") == 0)
    {
        return WaveformSaveToWAV(filename, wav,
                    status_callback_context, status_callback_func,
                    useFloat, useBytesPerSample);
    }
    else if (_wcsicmp(extension, L".mp3") == 0)
    {
        return WaveformSaveToMP3(filename, wav,
                    status_callback_context, status_callback_func);
    }
    else if (_wcsicmp(extension, L".raw") == 0 ||
             _wcsicmp(extension, L".pcm") == 0)
    {
        return WaveformSaveToRawPCM(filename, wav,
                    status_callback_context, status_callback_func,
                    useFloat, useBytesPerSample);
    }

    // Unrecognized filename extension!
#ifdef TRACE
    printf("Unrecognized filename extension on '%S'\n", filename);
#endif
    return false;
}

