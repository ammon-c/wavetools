//--------------------------------------------------------------------
// cmdopt.h
// C++ utility functions for examining command line arguments.
//
// (C) Copyright 1994-2017 Ammon R. Campbell
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
#include <ctype.h>

namespace {

//--------------------------------------------------------------------
// OptionNameIs:
// Checks if the name of the given command line argument has the
// specified name.  For example, if the argument string is
// "/OPTION=abc", it's name is "OPTION".  Returns true if the
// given name matches the given option string.  The comparison is
// not case sensitive.
//
// The template is intended to work for both "char" and "wchar_t"
// character types.
//--------------------------------------------------------------------
template<class T>
bool OptionNameIs(const T *szArg, const T *szName)
{
   if (szArg  == nullptr || szArg[0]  == '\0' ||
       szName == nullptr || szName[0] == '\0')
      return false;

   // If the command line argument is preceeded by a '-' or '/'
   // it indicates the start of an option switch.  Skip the prefix.
   if (szArg[0] == '-' || szArg[0] == '/')
      szArg++;

   // Compare name with beginning of argument string.
   // We're doing a case insensitive compare for purposes of option
   // name matching.
   while (*szArg && *szName)
   {
      if (tolower(*szName++) != tolower(*szArg++))
         return false;
   }

   // If the argument string ended before the name string, it's not
   // a match.
   if (*szName)
      return false;

   // If the next character of the argument string is alphanumeric,
   // then it's not a match.
   T c = *szArg;
   if (isalpha(c) || isdigit(c) || *szArg == '_')
      return false;

   return true;
}

//--------------------------------------------------------------------
// OptionValue:
// Retrieves a pointer to the value portion of the given command
// line argument string.  e.g. if the input string is
// "/OPTION=abc", the return pointer points to "abc".
// Returns a pointer to an empty string if the given string doesn't
// have a value attached.  A colon ':' or an equal symbol '=' may
// be used as a delimiter between the option's name and value in
// the given string.
//
// The template is intended to work for both "char" and "wchar_t"
// character types.
//--------------------------------------------------------------------
template<class T>
const T * OptionValue(const T *szArg)
{
   static T *p = L"";

   if (szArg == nullptr)
      return p;

   // Find the delimiter.
   while (*szArg && *szArg != '=' && *szArg != ':')
      szArg++;

   // Skip the delimiter.
   if (*szArg == '=' || *szArg == ':')
      return szArg + 1;

   return p;
}

} // End anon namespace
