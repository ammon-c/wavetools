//-------------------------------------------------------------------
//
// rawpcmfile.cpp
//
// C++ module to read and write raw PCM audio files.
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

#include "rawpcmfile.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define TRACE

// Handy class to auto-close a stdio FILE when it goes out of scope.
class ScopedFile
{
public:
    explicit ScopedFile(FILE *file) : m_file(file) { }
    ~ScopedFile() { Close(); }
    void Close() { if (m_file) fclose(m_file); m_file = nullptr; }

private:
    FILE *m_file;
};

// Retrieves the size of the given file in bytes.
// Returns zero if file couldn't be accessed.
uint64_t RawPCMFileGetSizeInBytes(const wchar_t *filename)
{
#ifdef TRACE
    printf("RawPCMFileGetSizeInBytes '%S'\n", filename);
#endif

    // Open the file for reading.
    FILE *fp = nullptr;
    if (_wfopen_s(&fp, filename, L"r+b") || !fp)
        return 0; // Can't open the file.
    ScopedFile sfp(fp);

    if (fseek(fp, 0, SEEK_END))
        return 0; // Seek failed.

    int64_t bytes = _ftelli64(fp);
    if (bytes < 1)
        return 0; // Error or empty file.

#ifdef TRACE
    printf("  bytes = %I64d\n", bytes);
#endif
    return static_cast<uint64_t>(bytes);
}

// Reads the audio samples from a raw PCM file into a caller
// provided buffer in memory.  Returns true if successful.
bool RawPCMFileRead(
    const wchar_t * filename,
    size_t          numSamples,
    unsigned        numChannels,
    unsigned        bytesPerSample,
    void *          buffer,
    size_t          bufferSize
    )
{
#ifdef TRACE
    printf("RawPCMFileRead '%S', bufferSize=%zu\n", filename, bufferSize);
    printf("  numSamples=%zu  numChannels=%u  bytesPerSample=%u\n",
        numSamples, numChannels, bytesPerSample);
#endif

    // Open the file for reading.
    FILE *fp = nullptr;
    if (_wfopen_s(&fp, filename, L"r+b") || !fp)
        return 0; // Can't open the file.
    ScopedFile sfp(fp);

    size_t bytesToRead = numSamples * numChannels * bytesPerSample;
    if (bytesToRead > bufferSize)
        return false;

    if (fread(buffer, 1, bytesToRead, fp) != bytesToRead)
        return false;

    return true;
}

// Writes the audio samples from a memory buffer to a raw PCM
// file.  Returns true if successful.
bool RawPCMFileWrite(
    const wchar_t * filename,
    size_t          numSamples,
    unsigned        numChannels,
    unsigned        bytesPerSample,
    void *          buffer
    )
{
#ifdef TRACE
    printf("RawPCMFileWrite '%S', buffer=%p\n", filename, buffer);
    printf("  numSamples=%zu  numChannels=%u  bytesPerSample=%u\n",
        numSamples, numChannels, bytesPerSample);
#endif

    // Open the file for writing.
    FILE *fp = nullptr;
    if (_wfopen_s(&fp, filename, L"w+b") || !fp)
    {
#ifdef TRACE
        printf("Failed opening '%S'\n", filename);
#endif

        return false; // Can't open the file.
    }
    ScopedFile sfp(fp);

    size_t bytesToWrite = numSamples * numChannels * bytesPerSample;

    size_t result = fwrite(buffer, 1, bytesToWrite, fp);
    if (result != bytesToWrite)
    {
#ifdef TRACE
        printf("Failed writing %zu bytes.  result=%zu fp=%p\n", bytesToWrite, result, fp);
#endif
        return false;
    }

    return true;
}

