//-------------------------------------------------------------------
//
// wavfile.cpp
//
// C++ module to read and write Microsoft WAV audio files.
// This is older code that has been modified several times over
// the years.  It isn't quite as shiny as new code would be.
//
// See wavfile.h for additional comments.
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

//#define TRACE // Define TRACE to enable debug printfs in this module.
#include "wavfile.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

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

#pragma pack(1)

// The file format header found within a WAV file.
typedef struct
{
    // Encoding: 1 = integer PCM data; 3 = floating-point PCM data.
    unsigned short int    wFmtTag;

    // Number of channels: 1 = mono; 2 = stereo.
    unsigned short int    nChannels;

    // Audio sampling rate in Hertz.
    unsigned long int    Rate;

    // Average bytes per second needed to output to play this sample.
    unsigned long int    BPS;

    // Number of bytes to output per sample.
    // For an 8-bit mono sample, this will be 1.
    // For an 8-bit stereo sample, this will be 2.
    // For a 16-bit mono sample, this will be 2.
    // For a 16-bit stereo sample, this will be 4.
    unsigned short int    nAlign;

    // Bits per sample (8 or 16 for integer PCM, 32 for floating-point).
    unsigned short int    nBits;
} WAVFHDR;

#pragma pack()

// Reads and verifies the signature at the beginning of a WAV file.
// Returns false if can't read from file or if file isn't a WAV file.
// Note that this function leaves the file pointer at the end of the
// 16-byte signature data in the open file.
static bool read_and_confirm_wav_signature(FILE *fp)
{
    // Read the 16-byte signature from the beginning of the file.
    char signature[16] = {0};
    if (fread(signature, 1, sizeof(signature), fp) != sizeof(signature))
    {
#ifdef TRACE
        printf("read_and_confirm_wav_signature: Can't read signature bytes!\n");
#endif
        return false; // Can't read signature bytes.
    }

    // Check that the signature is good.
    if (strncmp(signature, "RIFF", 4) != 0)
    {
#ifdef TRACE
        printf("read_and_confirm_wav_signature: Expected 'RIFF', got '%c%c%c%c'\n",
            signature[0], signature[1], signature[2], signature[3]);
#endif
        return false; // Doesn't appear to be a valid WAV file.
    }
    if (strncmp(&signature[8], "WAVE", 4) != 0)
    {
#ifdef TRACE
        printf("read_and_confirm_wav_signature: Expected 'WAVE', got '%c%c%c%c'\n",
            signature[8], signature[9], signature[10], signature[11]);
#endif
        return false; // Doesn't appear to be a valid WAV file.
    }
    if (strncmp(&signature[12], "JUNK", 4) == 0)
    {
        // There's a 'JUNK' block before the 'fmt ' block.  Skip it.
#ifdef TRACE
        printf("read_and_confirm_wav_signature: Skipping 'JUNK' block.\n");
#endif
        uint32_t offset = 0;
        fread(&offset, 1, 4, fp);
#ifdef TRACE
        printf("read_and_confirm_wav_signature: offset=%u\n", offset);
#endif
        fseek(fp, offset, SEEK_CUR);
#ifdef TRACE
        printf("read_and_confirm_wav_signature: ftell=%u\n", ftell(fp));
#endif
        fread(&signature[12], 1, 4, fp);
#ifdef TRACE
        printf("read_and_confirm_wav_signature: Next block '%c%c%c%c'.\n",
            signature[12], signature[13], signature[14], signature[15]);
#endif
    }
    if (strncmp(&signature[12], "fmt ", 4) != 0)
    {
#ifdef TRACE
        printf("read_and_confirm_wav_signature: Expected 'fmt ', got '%c%c%c%c'\n",
            signature[12], signature[13], signature[14], signature[15]);
#endif
        return false; // Doesn't appear to be a valid WAV file.
    }

#ifdef TRACE
    printf("read_and_confirm_wav_signature: Header appears OK.  ftell=%u\n", ftell(fp));
#endif
    return true;
}

// Reads and verifies the format header from a WAV file.  Assumes
// the file pointer is at the start of the header.  Returns true
// if successful.  Note this function leaves the file pointer at
// beginning of the next section of the open file if successful.
static bool read_and_confirm_format_header(FILE *fp, WAVFHDR &hdr)
{
    // Read the size of the format header.
    uint32_t hdr_size = 0;
    if (fread(&hdr_size, 1, sizeof(hdr_size), fp) != sizeof(hdr_size))
        return false; // Read error.
    if (hdr_size < sizeof(WAVFHDR))
        return false; // Invalid header size.
#ifdef TRACE
    printf("read_and_confirm_format_header: hdr_size=%u\n", hdr_size);
#endif

    // Read the format header.
    hdr = {0};
    if (fread(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr))
        return false; // Read error.

    // Check that the contents of the header are acceptable.
    if (hdr.nBits != 8 && hdr.nBits != 16 && hdr.nBits != 32)
        return false; // Unsupported format.
    if (hdr.wFmtTag != 1 && hdr.wFmtTag != 3)
        return false; // Unsupported format.
    if (hdr.nChannels < 1 || hdr.nChannels > 5)
        return false; // Unsupported format.

#ifdef TRACE
    printf("read_and_confirm_format_header: before seek ftell=%u\n", ftell(fp));
#endif

    // Seek past the header to the next chunk.
    if (fseek(fp, hdr_size - sizeof(hdr), SEEK_CUR))
        return false; // Seek failed.

#ifdef TRACE
    printf("read_and_confirm_format_header ok.  ftell=%u\n", ftell(fp));
#endif
    return true;
}

// Reads and verifies the "data" signature that appears at the
// beginning of the sample data chunk of the WAV file.  Assumes
// that the file pointer is already at the beginning of the next
// chunk in the WAV file when called.  If successful, populates
// 'datasize' with the number of bytes of data that follow the
// data header, and returns true, leaving the file pointer at
// the first byte of sample data.
static bool read_and_confirm_data_header(FILE *fp, uint32_t &datasize)
{
#ifdef TRACE
    printf("read_and_confirm_data_header: ftell=%u\n", ftell(fp));
#endif

    char datasig[4] = {0};

    // Keep reading chunks until we find the one that contains the
    // audio sample data.
    while (fread(datasig, 1, sizeof(datasig), fp) == sizeof(datasig))
    {
#ifdef TRACE
        printf("read_and_confirm_data_header: datasig='%c%c%c%c'\n",
            datasig[0], datasig[1], datasig[2], datasig[3]);
#endif

        // Read the size of this chunk of the WAV file.
        datasize = 0;
        if (fread(&datasize, 1, sizeof(datasize), fp) != sizeof(datasize))
        {
#ifdef TRACE
            printf("read_and_confirm_data_header: Failed getting chunk size!\n");
#endif
            return false;
        }
#ifdef TRACE
        printf("read_and_confirm_data_header: datasize=%u\n", datasize);
#endif

        // If the chunk's name is "data", we found what we're looking for.
        if (memcmp(datasig, "data", sizeof(datasig)) == 0)
        {
#ifdef TRACE
            printf("read_and_confirm_data_header: Found the 'data' chunk.\n");
#endif
            return true;
        }

        // Seek past this chunk's data bytes to the next chunk's header.
        if (fseek(fp, datasize, SEEK_CUR))
        {
#ifdef TRACE
            printf("read_and_confirm_data_header: Seek failed!\n");
#endif
            return false; // Seek failed.
        }
    }

    // Didn't find any "data" chunks in the rest of the WAV file.
    datasize = 0;

#ifdef TRACE
    printf("read_and_confirm_data_header: No 'data' chunk found!\n");
#endif

    return true;
}

// Reads the header portion of a WAV file.  Among other things, the
// information from the header can be used to determine how large
// of a sample buffer will be needed to read the audio data from the
// WAV file in a subsequent call to WAVFileReadSamples.
//
// Returns true if successful.
bool WAVFileReadHeader(const wchar_t *filename, WAVInfo &header)
{
#ifdef TRACE
    printf("WAVFileReadHeader file='%S'\n", filename);
#endif

    header = WAVInfo();

    if (!filename || !*filename)
    {
#ifdef TRACE
        printf("WAVFileReadHeader bad parameter!\n");
#endif
        return false; // Empty filename.
    }

    // Open the WAV file for reading.
    FILE *fp = nullptr;
    if (_wfopen_s(&fp, filename, L"r+b") || !fp)
    {
#ifdef TRACE
        printf("WAVFileReadHeader failed opening file '%S'\n", filename);
#endif
        return false; // Can't open the file.
    }
    ScopedFile sfp(fp);

    // Read and check the various headers in the WAV file.
    WAVFHDR hdr = {0};
    uint32_t datasize = 0;
    if (!read_and_confirm_wav_signature(fp))
    {
#ifdef TRACE
        printf("WAVFileReadHeader bad file signature, not a WAV file!\n");
#endif
        return false; // Unrecognized file signature, not a WAV.
    }
    if (!read_and_confirm_format_header(fp, hdr))
    {
#ifdef TRACE
        printf("WAVFileReadHeader unsupported audio format!\n");
#endif
        return false; // Unsupported audio format or read error.
    }
    if (!read_and_confirm_data_header(fp, datasize))
    {
#ifdef TRACE
        printf("WAVFileReadHeader data chunk not found or unreadable!\n");
#endif
        return false; // Data chunk not found or unreadable.
    }

    // Save a few pieces of info we'll need about the audio format.
    header.m_rate         = hdr.Rate;
    header.m_channels     = hdr.nChannels;
    header.m_bits         = hdr.nBits;
    header.m_is_float     = (hdr.wFmtTag == 3);
    header.m_sample_count = datasize / hdr.nChannels / (hdr.nBits / 8);

#ifdef TRACE
    printf("WAVFileReadHeader rate=%u nChannels=%u bits=%u float=%s samples=%u\n",
        header.m_rate, header.m_channels, header.m_bits,
        header.m_is_float ? "float" : "int", header.m_sample_count);
#endif
    return true;
}

// Reads the audio samples from a WAV file into the provided buffer.
// The buffer_size parameter should indicate the size limit of the
// buffer in bytes.
//
// Returns true if successful.
bool WAVFileReadSamples(const wchar_t *filename, void *sample_buffer, size_t buffer_size)
{
#ifdef TRACE
    printf("WAVFileReadSamples file='%S'\n", filename);
#endif

    if (!filename || !*filename || !sample_buffer || !buffer_size)
        return false; // Bad parameter.

    // Open the WAV file for reading.
    FILE *fp = nullptr;
    if (_wfopen_s(&fp, filename, L"r+b") || !fp)
        return false;
    ScopedFile sfp(fp);

    // Read and check the various headers in the WAV file.
    WAVFHDR hdr = {0};
    uint32_t data_size = 0;
    if (!read_and_confirm_wav_signature(fp))
        return false; // Unrecognized file signature, not a WAV.
    if (!read_and_confirm_format_header(fp, hdr))
        return false; // Unsupported audio format or read error.
    if (!read_and_confirm_data_header(fp, data_size))
        return false; // Data chunk not found or unreadable.

    // Read the sample data into the caller's buffer.
    if (buffer_size < data_size)
        return false; // Buffer is too small.
    if (fread(sample_buffer, 1, data_size, fp) != data_size)
        return false;

    return true;
}

// Writes a buffer of audio samples to a WAV file.
// The given header specifies the format of the data in the buffer.
//
// Returns true if successful.
bool WAVFileWrite(const wchar_t *filename, const WAVInfo &header, const void *samples)
{
    if (!filename || !*filename || !samples || !header.m_sample_count)
        return false; // Bad parameter.
    if (header.m_bits != 8 && header.m_bits != 16 && header.m_bits != 32)
        return false;

#ifdef TRACE
    printf("WAVFileWrite file='%S'\n", filename);
#endif

    // Open the WAV file for writing.
    FILE *fp = nullptr;
    if (_wfopen_s(&fp, filename, L"w+b") || !fp)
        return false;
    ScopedFile sfp(fp);

    // Write the file signature.
    uint32_t offset = (uint32_t)(16 + sizeof(WAVFHDR) + header.CalculateBufferSize());
    if (fwrite("RIFF", 1, 4, fp) != 4)
        return false;
    if (fwrite(&offset, 1, sizeof(offset), fp) != sizeof(offset))
        return false;
    if (fwrite("WAVEfmt ", 1, 8, fp) != 8)
        return false;

    // Write the size of the format header.
    uint32_t hdr_size = static_cast<uint32_t>(sizeof(WAVFHDR));
    if (fwrite(&hdr_size, 1, sizeof(hdr_size), fp) != sizeof(hdr_size))
        return false;

    // Write the format header.
    WAVFHDR wfhdr = {0};
    wfhdr.wFmtTag   = (header.m_is_float ? 3 : 1);
    wfhdr.nChannels = static_cast<unsigned short>(header.m_channels);
    wfhdr.Rate      = header.m_rate;
    wfhdr.BPS       = header.m_rate * (header.m_bits / 8);
    wfhdr.nAlign    = static_cast<unsigned short>(header.m_bits / 8 * header.m_channels);
    wfhdr.nBits     = static_cast<unsigned short>(header.m_bits);
    if (fwrite(&wfhdr, 1, sizeof(wfhdr), fp) != sizeof(wfhdr))
        return false;

    // Write the header for the "data" chunk.
    uint32_t data_size = header.CalculateBufferSize();
    if (fwrite("data", 1, 4, fp) != 4)
        return false;
    if (fwrite(&data_size, 1, sizeof(data_size), fp) != sizeof(data_size))
        return false;

    // Write the raw sample data.
    if (fwrite(samples, 1, data_size, fp) != data_size)
        return false;

    return true;
}

