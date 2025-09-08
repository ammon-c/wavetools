## README for WaveTools - Utilities for manipulating audio files

Author: Ammon Campbell (user *ammon-c* on Github)

**What Is WaveTools?**

*WaveTools* is a collection of command-line utilities for
manipulating digital audio files.  Tools are provided for a
variety of tasks such as converting, extending, joining,
trimming, stretching, resampling, and adding effects to audio
waveforms.  

I wrote these tools for my own use.  You may also use them, but
at your own risk.  Refer the [license](#tagLicense) section at
the end of this document for important legal notices.  

**Supported File Formats:**

*WaveTools* supports the following audio file formats:

* Microsoft WAV (read & write)
* Raw PCM (read & write)
* MP3 (read only)

Optionally, *WaveTools* can also write MP3 file format with the
aid of another software tool called *ffmpeg* (see below).  

**Platform:**

* *WaveTools* compiles with Microsoft Visual Studio 2022, and
runs on Windows 10 or 11 (64-bit).  

**Limitations:**

* *WaveTools* doesn't support audio files that are too large to
fit in system memory.  

* *WaveTools* doesn't support reading of compressed WAV files. 
Uncompressed WAV files are generally supported.  

---

### Table of Contents

* [WaveCompare Utility](#tagWaveCompare) - Compare two audio files
* [WaveConvert Utility](#tagWaveConvert) - Convert audio file to a different format
* [WaveEcho Utility](#tagWaveEcho) - Add echo effect to an audio file
* [WaveEQ Utility](#tagWaveEQ) - Apply equalization filters to an audio file
* [WaveExtend Utility](#tagWaveExtend) - Add silence at the start/end of an audio file
* [WaveFade Utility](#tagWaveFade) - Add fade-in/fade-out effect to an audio file
* [WaveGate Utility](#tagWaveGate) - Apply noise gate filter to an audio file
* [WaveInfo Utility](#tagWaveInfo) - Display information about audio files
* [WaveJoin Utility](#tagWaveJoin) - Concatenate multiple audio files into one
* [WaveNormalize Utility](#tagWaveNormalize) - Normalize the volume level in an audio file
* [WaveMix Utility](#tagWaveMix) - Mix multiple audio files into one
* [WavePrint Utility](#tagWavePrint) - Print a text-based graph of an audio file
* [WaveRate Utility](#tagWaveRate) - Change the sampling rate of an audio file (resample audio)
* [WaveStretch Utility](#tagWaveStretch) - Stretch or shrink (slow down or speed up) an audio file
* [WaveTremolo Utility](#tagWaveTremolo) - Apply a tremolo effect to an audio file
* [WaveTrim Utility](#tagWaveTrim) - Delete a portion of an audio file
* [WaveVibrato Utility](#tagWaveVibrato) - Apply a vibrato effect to an audio file
* [WaveVolume Utility](#tagWaveVolume) - Change the volume of an audio file

- [Source Code](#tagSourceCode)
- [Build](#tagBuild)
- [What about binary releases?](#tagReleases)
- [Tests](#tagTests)
- [Enable writing of MP3 file format by using FFMPEG](#tagFfmpeg)
- [To Do / Wish List](#tagToDo)
- [License](#tagLicense)

---
<a name="tagWaveCompare"></a>

### WaveCompare Utility

**WaveCompare** compares two audio files to see if they contain
the same or substantially similar waveform.

The program exit code is zero if the two files appear to match,
or one if they don't match or can't be loaded.

```
Usage:  wavecompare [options] file1 file2

Options:
  -Threshold=x : If the average difference between the samples 
                 in the two waveforms exceeds value 'x', the 
                 waveforms are considered to be different from 
                 each other.  0.001 is the default.
```

**Example Output:**

```
---
Comparing 'file1.wav' and 'file2.wav' with threshold 0.00100000
---
Loaded 'file1.wav', length 15.67 seconds.
Loaded 'file2.wav', length 15.67 seconds.
Sum of differences:        9.060260
Average difference:        0.000002
Threshold:                 0.001000
Comparison:  Yes, 'file1.wav' and 'file2.wav' appear to be a match.
```

---
<a name="tagWaveConvert"></a>

### WaveConvert Utility

**WaveConvert** reads an existing audio file and writes the
audio data to a new audio file, possibly in a different file
format or with a different sample encoding format.  

```
Usage:  waveconvert [options] infile outfile

Options:
  -Mono : Convert the audio to mono (one channel).

  -Stereo : Convert the audio to stereo (two channels).

  -Float=x : For file formats that support both integer and 
       floating-point samples, this indicates which to 
       use, where 'x' may be 'yes' or 'no'. 

  -BytesPerSample=x : For file formats that support multiple 
       sample sizes, this indicates which sample size to use, 
       where 'x' is typically 1, 2, or 4 for integer samples, 
       and 4 or 8 for floating-point samples. 
```

---
<a name="tagWaveEcho"></a>

### WaveEcho Utility

**WaveEcho** reads an audio file, adds an echo effect to
the audio, and writes the altered waveform to a new file.  

```
Usage:  waveecho [options] delay repeat infile outfile

Where:
  delay : Indicates the delay time in milliseconds. 

  repeat : Indicates the repeat count for the echo.

Options:
  -WetLevel=x : Specify how much wet signal to include in the 
       altered waveform, as a floating-point number between 0 
       and 1.  Default is 0.5.

  -DryLevel=x : Specify how much dry signal to include in the 
       altered waveform, as a floating-point number between 0 
       and 1.  Default is 1.

  -Float=x : For file formats that support both integer and 
       floating-point samples, this indicates which to use 
       when writing 'outfile', where 'x' may be 'yes' or 'no'. 

  -BytesPerSample=x : For file formats that support multiple 
       sample sizes, this indicates which sample size to use 
       when writing 'outfile', where 'x' is typically 1, 2, or 
       4 for integer samples, and 4 or 8 for floating-point 
       samples. 
```

---
<a name="tagWaveEQ"></a>

### WaveEQ Utility

**WaveEQ** reads an audio file, applies low-pass, high-pass,
notch and/or bandpass filtering to the audio signal, and
writes the altered waveform to a new file.  

```
Usage:  waveeq [options] lowpass highpass infile outfile

Where:
  lowpass  ..is the low pass (bass cutoff) frequency in Hertz. 
             Or zero to disable low-pass filtering. 
  highpass ..is the high pass (treble cutoff) frequency in Hertz. 
             Or zero to disable high-pass filtering. 

Options:
  -BandPassFreq=x : Applies a bandpass filter with the specified 
       center frequency. 

  -BandPassQ=x : Specifies the Q-factor parameter for the bandpass 
       filter.  Ignored if -BandPassFreq option is not used. 

  -NotchFreq=x : Applies a notch filter with the specified 
       center frequency. 

  -NotchQ=x : Specifies the Q-factor parameter for the notch 
       filter.  Ignored if -NotchFreq option is not used. 

  -Float=x : For file formats that support both integer and 
       floating-point samples, this indicates which to use 
       when writing 'outfile', where 'x' may be 'yes' or 'no'. 

  -BytesPerSample=x : For file formats that support multiple 
       sample sizes, this indicates which sample size to use 
       when writing 'outfile', where 'x' is typically 1, 2, or 
       4 for integer samples, and 4 or 8 for floating-point 
       samples. 
```

---
<a name="tagWaveExtend"></a>

### WaveExtend Utility

**WaveExtend** reads an existing audio file, adds silence to the
beginning and/or ending of the waveform, and writes the altered
waveform to a new file.  

```
Usage:  wavestextend [options] before after infile outfile

Where:
  before : Indicates how many samples to add to the beginning 
       of the waveform.  Or if '-UseTime' is also given, indicates 
       how many seconds to add to the beginning of the waveform. 

  after : Indicates how many samples to add to the end of the 
       waveform.  Or if '-UseTime' is also given, indicates how 
       many seconds to add to the beginning of the waveform. 

Options:
  -UseTime : Indicates that the 'before' and 'after' parameters 
       are measured in seconds rather than number of samples. 

  -Float=x : For file formats that support both integer and 
       floating-point samples, this indicates which to use 
       when writing 'outfile', where 'x' may be 'yes' or 'no'. 

  -BytesPerSample=x : For file formats that support multiple 
       sample sizes, this indicates which sample size to use 
       when writing 'outfile', where 'x' is typically 1, 2, or 
       4 for integer samples, and 4 or 8 for floating-point 
       samples. 
```

---
<a name="tagWaveFade"></a>

### WaveFade Utility

**WaveFade** reads an audio file, applies a fade-in to the
beginning of the audio and/or applies a fade-out to the end
of the audio, and writes the altered waveform to a new
file.  

```
Usage:  wavefade [options] fadein fadeout infile outfile

Where:
  fadein  ..is a number indicating the duration of the fade-in 
            in seconds.  May be zero for no fade-in. 

  fadeout ..is a number indicating the duration of the fade-out 
            in seconds.  May be zero for no fade-out. 

  infile  ..is the name of the audio file to read. 

  outfile ..is the name of the file to which the altered audio
            will be written. 

Options:
  -Float=x : For file formats that support both integer and 
       floating-point samples, this indicates which to use 
       when writing 'outfile', where 'x' may be 'yes' or 'no'. 

  -BytesPerSample=x : For file formats that support multiple 
       sample sizes, this indicates which sample size to use 
       when writing 'outfile', where 'x' is typically 1, 2, or 
       4 for integer samples, and 4 or 8 for floating-point 
       samples. 

```

---
<a name="tagWaveGate"></a>

### WaveGate Utility

**WaveGate** reads an audio file, applies a noise gate filter, and
writes the altered waveform to a new file.

```
Usage:  wavegate [options] infile outfile

Options:
  -Threshold=x : Specifies the gate threshold, where 'x' is a 
       sample level between 0.0000001 and 1.  Default is 0.1. 

  -TrimStart : Removes the silence at the beginning of the 
       waveform, if any. 

  -TrimEnd : Removes the silence at the end of the waveform, 
       if any. 

  -Float=x : For file formats that support both integer and 
       floating-point samples, this indicates which to use 
       when writing 'outfile', where 'x' may be 'yes' or 'no'. 

  -BytesPerSample=x : For file formats that support multiple 
       sample sizes, this indicates which sample size to use 
       when writing 'outfile', where 'x' is typically 1, 2, or 
       4 for integer samples, and 4 or 8 for floating-point 
       samples. 

```

---
<a name="tagWaveInfo"></a>

### WaveInfo Utility

**WaveInfo** shows general information about one or more audio
files.

```
Usage:  waveinfo file1.wav [file2.wav ...]
```

**Example Output:**

```
Processing 'recording.wav'
Samples:    691200
Rate:       44100 Hz
Channels:   1
Duration:   15.67 seconds
FPCM Bytes: 691200
Highest sample:  0.52
Lowest sample:  -0.62
Completed OK.
```

---
<a name="tagWaveJoin"></a>

### WaveJoin Utility

**WaveJoin** Creates a new audio file by joining multiple audio
files together sequentially (one after another).  

```
Usage:  wavejoin [options] infile1 infile2 [infile3 ...] outfile

Options:

  -Float=x : For file formats that support both integer and 
       floating-point samples, this indicates which to use 
       when writing 'outfile', where 'x' may be 'yes' or 'no'. 

  -BytesPerSample=x : For file formats that support multiple 
       sample sizes, this indicates which sample size to use 
       when writing 'outfile', where 'x' is typically 1, 2, or 
       4 for integer samples, and 4 or 8 for floating-point 
       samples. 

```


---
<a name="tagWaveNormalize"></a>

### WaveNormalize Utility

**WaveNormalize** Reads an audio file, normalizes the audio
level of the samples in the waveform, and writes the altered
waveform to a new audio file.  

```
Usage:  wavenormalize [options] dbLevel infile outfile

Where:
  dbLevel : Indicates the decibel level to which the audio 
       samples are to be normalized.  The value of dbLevel 
       should be between 0 (loudest) and -100 (quietest). 
       -1 is recommended level for most applications.

Options:
  -Float=x : For file formats that support both integer and 
       floating-point samples, this indicates which to use 
       when writing 'outfile', where 'x' may be 'yes' or 'no'. 

  -BytesPerSample=x : For file formats that support multiple 
       sample sizes, this indicates which sample size to use 
       when writing 'outfile', where 'x' is typically 1, 2, or 
       4 for integer samples, and 4 or 8 for floating-point 
       samples. 
```


---
<a name="tagWaveMix"></a>

### WaveMix Utility

**WaveMix** Mixes multiple audio files together and writes
the mixed audio to a new audio file.  

```
Usage:  wavemix [options] outfile infile[,volume[,start]] [infile2...]

Where:
  outfile : Indicates the name of the file to which the mixed 
            audio waveform will be written. 

  infile : Indicates the name of an audio file to be mixed. 

  volume : Indicates the volume level of 'infile' in the mix. 
           The level is between 0 and 1.  Default is 0.5. 

  start : Indicates the time offset in the output file where 
          'infile' should start being mixed in, in seconds. 
          Default is 0. 

Options:
  -Float=x : For file formats that support both integer and 
       floating-point samples, this indicates which to use 
       when writing 'outfile', where 'x' may be 'yes' or 'no'. 

  -BytesPerSample=x : For file formats that support multiple 
       sample sizes, this indicates which sample size to use 
       when writing 'outfile', where 'x' is typically 1, 2, or 
       4 for integer samples, and 4 or 8 for floating-point 
       samples. 
```


---
<a name="tagWavePrint"></a>

### WavePrint Utility

**WavePrint** Reads an audio file and prints a graph of the
waveform amplitude over time to the console.

```
Usage:  waveprint [options] filename

Options:
  -UseTime : Indicates that the numbers used for the -Count, 
       -Start, and -PerLine options are specified in seconds
       rather than sample index numbers. 

  -Count=x : Indicates the number of samples to print.  Or the 
       number of seconds to print if -UseTime is also specified. 
       If -Count is not given, printing will continue until the 
       end of the waveform. 

  -Start=x : Indicates the sample number where printing will 
       start.  Or the starting position in seconds if -UseTime 
       is also specified. 

  -PerLine=x : Indicates the number of samples represented by each 
       line in the printed graph.  Or the number of seconds for 
       each line of -UseTime is also specified. 

  -Width=x : Indicates the width of the printed graph in 
       character cells.  Default is 60.

  -Min=x : Indicates the amplitude represented by the left edge 
       of the graph.  Default is -1.0.

  -Max=x : Indicates the amplitude represented by the right edge 
       of the graph.  Default is 1.0.
```

**Example Output:**

```
WavePrint:  Settings:
  Printing 'recording.wav'
  Start at sample:    0
  Number of samples:  All
  Samples per line:   10000
  Amplitude range:    Min:-1, Max:1
  Terminal width:     60 characters
WavePrint:  Loaded 691200 samples (15.6735 seconds) from '..\testdata\airhost.wav' at 44100 Hz

        -1                                                 1
         +-------------------------------------------------+
       0 |                        #                        |
   10000 |               ##################                |
   20000 |              ####################               |
   30000 |            ######################               |
   40000 |                  #############                  |
   50000 |                  ############                   |
   60000 |                        #                        |
   70000 |                        #                        |
   80000 |                ################                 |
   90000 |                 ##############                  |
  100000 |           ##########################            |
  110000 |                 ###############                 |
  120000 |              ####################               |
  130000 |                 #############                   |
  140000 |                   ############                  |
  150000 |                    #########                    |
  160000 |                        #                        |
  170000 |                        #                        |
  180000 |                     ######                      |
  190000 |              ####################               |
  200000 |            #####################                |
  210000 |         ############################            |
  220000 |                 #############                   |
  230000 |               #######################           |
  240000 |            #######################              |
  250000 |               #################                 |
  260000 |                 #############                   |
  270000 |                        #                        |
  280000 |                       ####                      |
  290000 |             #####################               |
  300000 |                 ############                    |
  310000 |                      ####                       |
  320000 |                        #                        |
  330000 |                        #                        |
  340000 |                        #                        |
  350000 |                        #                        |
  360000 |                        #                        |
  370000 |                        #                        |
  380000 |             #####################               |
  390000 |             #######################             |
  400000 |             #######################             |
  410000 |                #################                |
  420000 |                 ##############                  |
  430000 |                       ###                       |
  440000 |                  #############                  |
  450000 |             ####################                |
  460000 |             ####################                |
  470000 |             ####################                |
  480000 |             ####################                |
  490000 |               ###################               |
  500000 |                   ##########                    |
  510000 |                     ######                      |
  520000 |                       ##                        |
  530000 |              ###################                |
  540000 |              #####################              |
  550000 |              ####################               |
  560000 |                 ##############                  |
  570000 |                 ############                    |
  580000 |             ####################                |
  590000 |                ################                 |
  600000 |                   ##########                    |
  610000 |                   ##########                    |
  620000 |                       ###                       |
  630000 |                      ####                       |
  640000 |              ###################                |
  650000 |                  ###########                    |
  660000 |                     ######                      |
  670000 |                        #                        |
  680000 |                        #                        |
  690000 |                        #                        |
         +-------------------------------------------------+
        -1                                                 1
```

---
<a name="tagWaveRate"></a>

### WaveRate Utility

**WaveRate** reads an existing audio file, resamples the audio
data to a new sample rate, and writes the resampled audio data
to a new file.  

```
Usage:  waverate [options] rate infile outfile

Options:
  -Float=x : For file formats that support both integer and 
       floating-point samples, this indicates which to use 
       when writing 'outfile', where 'x' may be 'yes' or 'no'. 

  -BytesPerSample=x : For file formats that support multiple 
       sample sizes, this indicates which sample size to use 
       when writing 'outfile', where 'x' is typically 1, 2, or 
       4 for integer samples, and 4 or 8 for floating-point 
       samples. 

  -Help : Print this usage information to the console.

  -License : Print the copyright notice and software license 
       information to the console.
```

---
<a name="tagWaveStretch"></a>

### WaveStretch Utility

**WaveStretch** reads an existing audio file, stretches or
shrinks the audio waveform by a given multiplier parameter, and
writes the altered waveform to a new audio file.  If the
multiplier is less than one, the audio plays back faster, and
the perceived pitch of the audio is higher than the original
audio.  If the multiplier is more than one, the audio plays back
slower, and the perceived pitch of the audio is lower than the
original audio.  

```
Usage:  wavestretch multiplier infile outfile

The duration of the audio in the output file is calculated 
by multiplying the duration of the input file by the given
multiplier parameter.

Examples: 

  * Double the length of a waveform (slow it down by 2x):
      wavestretch 2.0 one.wav two.wav

  * Halve the length of a waveform (speed it up by 2x):
      wavestretch 0.5 one.wav two.wav
```

---
<a name="tagWaveTremolo"></a>

### WaveTremolo Utility

**WaveTremolo** applies a tremolo filter to the waveform from an
audio file and writes the filtered waveform to a new file.  

```
Usage:  wavetremolo [options] width depth infile outfile

Where:
  width : Indicates the width of each tremolo pulsation in 
       samples.  Or if '-useTime' is also given, indicates the 
       width in seconds. 

  depth : Indicates the depth of the tremolo effect, from 0 to 1.

Options:
  -UseTime : Indicates that the 'before' and 'after' parameters 
       are measured in seconds rather than number of samples. 

  -Float=x : For file formats that support both integer and 
       floating-point samples, this indicates which to use 
       when writing 'outfile', where 'x' may be 'yes' or 'no'. 

  -BytesPerSample=x : For file formats that support multiple 
       sample sizes, this indicates which sample size to use 
       when writing 'outfile', where 'x' is typically 1, 2, or 
       4 for integer samples, and 4 or 8 for floating-point 
       samples. 
```

---
<a name="tagWaveTrim"></a>

### WaveTrim Utility

**WaveTrim** reads an existing audio file, deletes a portion of
the audio, and write the altered waveform to a new file.  

```
Usage:  wavetrim [options] infile outfile

Options:
  -UseTime : Indicates that the numbers used for the -Count 
       and -Start options are specified in seconds rather than 
       sample numbers. 

  -Count=x : Indicates the number of samples to delete.  Or the 
       number of seconds to delete if -UseTime is also specified. 

  -Start=x : If 'x' is a number, it indicates the sample number 
       where deletion will start.  Or the starting position in 
       seconds if -UseTime is also specified.  If 'x' is the word 
       END instead of a number, it indicates that deletion will 
       trim backward from the end of the waveform. 

  -Invert : Instead of deleting the indicated part of the waveform, 
       deletes everything except the indicated part. 

  -Float=x : For file formats that support both integer and 
       floating-point samples, this indicates which to use 
       when writing 'outfile', where 'x' may be 'yes' or 'no'. 

  -BytesPerSample=x : For file formats that support multiple 
       sample sizes, this indicates which sample size to use 
       when writing 'outfile', where 'x' is typically 1, 2, or 
       4 for integer samples, and 4 or 8 for floating-point 
       samples. 

```

---
<a name="tagWaveVibrato"></a>

### WaveVibrato Utility

**WaveVibrato** reads an audio file, applies a vibrato
effect to all samples in the waveform, and writes the
altered waveform to a new audio file.  By adjusting the
values of the width, depth, wetlevel, and drylevel
parameters, a variety of vibrato-based effects can be
produced; for example, flanging, phasing, and tape
warbling.  

```
Usage:  wavevibrato [options] width depth infile outfile

Where:
  width : Indicates the width of the vibrato effect in seconds 
          per cycle. 

  depth : Indicates the depth of the vibrato effect in milliseconds. 

Options:
  -WetLevel=x : Specify how much wet signal to include in the 
       altered waveform, as a floating-point number between 0 
       and 1.  Default is 1.

  -DryLevel=x : Specify how much dry signal to include in the 
       altered waveform, as a floating-point number between 0 
       and 1. Default is 0.

  -Float=x : For file formats that support both integer and 
       floating-point samples, this indicates which to use 
       when writing 'outfile', where 'x' may be 'yes' or 'no'. 

  -BytesPerSample=x : For file formats that support multiple 
       sample sizes, this indicates which sample size to use 
       when writing 'outfile', where 'x' is typically 1, 2, or 
       4 for integer samples, and 4 or 8 for floating-point 
       samples. 
```

---
<a name="tagWaveVolume"></a>

### WaveVolume Utility

**WaveVolume** reads an audio file, applies a volume multiplier
to all samples in the waveform, and writes the altered waveform
to a new audio file.

```
Usage:  wavevolume [options] volumeMultiplier infile outfile

Options:
  -Float=x : For file formats that support both integer and 
       floating-point samples, this indicates which to use 
       when writing 'outfile', where 'x' may be 'yes' or 'no'. 

  -BytesPerSample=x : For file formats that support multiple 
       sample sizes, this indicates which sample size to use 
       when writing 'outfile', where 'x' is typically 1, 2, or 
       4 for integer samples, and 4 or 8 for floating-point 
       samples. 
```

---
<a name="tagSourceCode"></a>

### Source Code

*WaveTools* is mostly written in C++.  The *WaveTools*
source code is organized into several subdirectories, as
follows:  

* [**include**](include) :  This directory contains public C++
header files for *waveformlib*, a static-link library that
contains the bulk of the shared waveform management code that is
used by each of the utility programs in *WaveTools*.  These
header files may also be included in user-created C++ programs
that wish to use the APIs in *waveformlib*.  

* [**libsrc**](libsrc) :  This directory contains the C++ source
code for *waveformlib*, a static-link library that contains the
bulk of the shared waveform management code that is used by each
of the utility programs in *WaveTools*.  This library may also be
linked to user-created C++ programs that wish to use the APIs in
*waveformlib*.  

* [**dependencies/minimp3**](dependencies/minimp3) :  This
directory contains the C++ header files from **minimp3**,
an open source project for reading MP3 audio files.  The
minimp3 project may be found on Github at this link: 
https://github.com/lieff/minimp3 .  

* [**subsys**](subsys) :  This directory contains several
stand-alone C++ modules that are called from *waveformlib*
and/or the *WaveTools* utility programs.  The bulk of this code
is related to low-level reading and writing of different audio
file formats, and to equalization filters for audio data. 

* [**test**](test) :  This directory contains testing scripts
and the C++ code for the unit tests.  

* [**testdata**](testdata) :  This directory contains several
digital audio files that are used by the tests.  

* [**tools**](tools) :  This directory contains the C++ code for
the *WaveTools* utility programs (WaveInfo, WaveConvert, etc.)

Some important source files:

* [**makefile**](makefile) :  A script for Microsoft NMAKE that
builds the *WaveTools* binaries from the source code.  

* [**include/waveform.h**](include/waveform.h) :  C++ header
file to include in programs that use the *Waveform* class.  

* [**include/waveformload.h**](include/waveformload.h) :  C++
header file to include in programs that read audio files into
*Waveform* objects.  

* [**include/waveformsave.h**](include/waveformsave.h) :  C++
header file to include in programs that write *Waveform* objects
to audio files.  

---
<a name="tagBuild"></a>

### Build

* It is assumed that Microsoft Visual Studio 2022 is installed,
and the Microsoft tools are configured to be usable from the
Windows command prompt.  

* At a Windows command prompt, **CD** to the directory where you
have placed the *WaveTools* files.  

* For a normal (aka release) build, run **NMAKE** at the command prompt.  

* For a debug build, run **NMAKE DEBUG=1** at the command prompt.

* If the build is successful, the compiled binaries are
located in the **x64/Debug** or **x64/Release** output
directory.

* There is no installer or automated deployment for this
software.  After building, you will probably want to either
copy the .exe files from the output directory to another
directory that is already named in your PATH environment
variable, or, alter your PATH to include the output
directory.  

---
<a name="tagReleases"></a>

### What about binary releases?

Sorry, binary releases of WaveTools are not provided here. 
This software is intended to be built from the source code
using a C++ compiler.  

---
<a name="tagTests"></a>

### Tests

After the software has been built, there are several groups of
tests that can be run to confirm that the software is working as
intended.  The tests may be run from a command prompt in the
[**test**](test) directory, using the collection of Windows
batch scripts located there.  At the Windows command prompt,
**CD** into the **test** directory before running any of the
scripts.  

* **Available test scripts:**  RunUnitTests.bat,
RunWaveCompareTests.bat, RunWaveConvertTests.bat,
RunWaveExtendTests.bat, RunWaveFadeTests.bat, RunWaveGateTests.bat,
RunWaveInfoTests.bat, RunWaveJoinTests.bat,
RunWaveNormalizeTests.bat, RunWavePrintTests.bat,
RunWaveRateTests.bat, RunWaveStretchTests.bat,
RunWaveTremoloTests.bat, RunWaveTrimTests.bat,
RunWaveVolumeTests.bat, CleanTests.bat 

Each test script writes console output to a log file.  For
example, **RunWaveCompareTests.bat** writes its log to
**WaveCompareTests.out**, and **RunWavePrintTests.bat** writes
its lot to **WavePrintTests.out**, and so on.  When a test
script has finished running, you may examine the log file for
details of the tests that were just run, including any test
failures that may have occurred.  

Many of the test scripts will create audio files in the **test**
directory as part of the testing procedure.  The filenames of
these files are prefixed with "**testout_**" (for example,
**testout_company.wav**).  After the script has run, you can
listen to these audio files (using any media player program) to
confirm that the audio generated by the test sounds the way it
is supposed to sound for that particular test.  

When you are done examining the test output files (log files and
audio files) that were created by any of the test scripts, you
may run the **CleanTests.bat** script to delete the test output
files from the **test** directory. 

---
<a name="tagFfmpeg"></a>

### Enable writing of MP3 file format by using FFMPEG

*WaveTools* can read MP3 files, but it doesn't know how to write
MP3 files by itself.  However, with the assistance of another
software tool called *ffmpeg*, it becomes possible for
*WaveTools* to write MP3 files.  

To enable writing of MP3 files in *WaveTools*, do the following:  

* Download a recent release of ffmpeg for Windows and find the
**ffmpeg.exe** file.  The FFMPEG official web site is here: 
https://www.ffmpeg.org/download.html 

* Put a copy of the **ffmpeg.exe** file into the directory where
your *WaveTools* executables are located.  

---
<a name="tagToDo"></a>

### To Do / Wish List

Some things I want to add to WaveTools someday:  

* Add support for reading/writing more kinds of audio files (FLAC, OGG, AAC, AIFF, WMA, etc).

  * It would be really nice to have an mp3 writer that doesn't depend on ffmpeg or some other GPL-encumbered tool.

* For raw PCM audio files, add support for an optional .hdr parameters file to accompany the raw audio file and describe the sample rate, channel count, etc.

* When converting from mono to stereo, allow pan position to be specified (or left/right volumes to be specified).

* Add more tools:

  * Add tools to extract audio waveforms from video files (AVI, MP4, WMV, etc).
  * Add tools to extract audio waveforms from .mod and .s3m music files.
  * WaveReverb?  Add a tool to generate reverberation effects in an audio file.
  * WaveClipper?  Add a tool to clip/saturate/distort an audio waveform.
  * WavePlay?  Add a command line audio player.
  * WavePlot?  Add a tool to draw a waveform to a bitmap file (.BMP/.JPG/etc).

* Make WaveTools work on Linux.

* Make WaveTools work on macOS.

* Add more tests.  Add better tests.

* Add a deployment script or installer.

---
<a name="tagLicense"></a>

### License

```
(C) Copyright 1994-2025 by Ammon R. Campbell.

I wrote this code for use in my own educational and experimental
programs, but you may also freely use it in yours as long as you
abide by the following terms and conditions:

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials
    provided with the distribution.
  * The name(s) of the author(s) and contributors (if any) may not
    be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.  IN OTHER WORDS, USE AT YOUR OWN RISK, NOT OURS.  
```

-*- end -*-
