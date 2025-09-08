//-------------------------------------------------------------------
//
// waveformsave.h
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

#pragma once
#include <vector>
#include <cstdint>
#include "waveform.h"

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
        void *status_callback_context = nullptr,
        bool (*status_callback_func)(void *context, float completion) = nullptr,

        // For audio file formats that support both integer and
        // floating-point data, this indicates which to write.
        bool useFloat = false,

        // For audio file formats that support more than one
        // possible size for audio samples, this indicates which
        // size to write.
        unsigned useBytesPerSample = 2
        );

