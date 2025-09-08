#
# Makefile to build WaveTools audio utilities on Windows.
# Requires Microsoft Visual Studio 2022 with NMAKE.
#
# To build in release mode:
#    NMAKE
#
# To build in debug mode:
#    NMAKE DEBUG=1
#
# Debug binaries live in "x64\Debug" subdirectory.
# Release binaries live in "x64\Release" subdirectory.
#
# Note that the release build uses -Ox optimization,
# because it's very slow with optimization disabled.
#

!ifndef DEBUG
OBJDIR=x64\ReleaseObj
BINDIR=x64\Release
CPPFLAGS= -nologo -c -Gs -EHsc -W4 -WX -I./subsys -I./include -DWIN32 -D_WIN32 -MT -Ox
!else
OBJDIR=x64\DebugObj
BINDIR=x64\Debug
CPPFLAGS= -nologo -c -Gs -EHsc -W4 -WX -I./subsys -I./include -DWIN32 -D_WIN32 -D_DEBUG -MTd -Ox -Zi
!endif

HDRS= include/waveform.h include/waveformload.h include/waveformsave.h \
      subsys/cmdopt.h subsys/wavfile.h subsys/rawpcmfile.h \
      subsys/lowpass.h subsys/highpass.h subsys/notchfilter.h \
      tools/notice.h dependencies/minimp3/minimp3.h

.SUFFIXES: .cpp

{.}.cpp{$(OBJDIR)}.obj:
   cl $(CPPFLAGS) -Fo$*.obj -Fd$(OBJDIR)\vc140.pdb $<

{./libsrc}.cpp{$(OBJDIR)}.obj:
   cl $(CPPFLAGS) -Fo$*.obj -Fd$(OBJDIR)\vc140.pdb $<

{./subsys}.cpp{$(OBJDIR)}.obj:
   cl $(CPPFLAGS) -Fo$*.obj -Fd$(OBJDIR)\vc140.pdb $<

{./test}.cpp{$(OBJDIR)}.obj:
   cl $(CPPFLAGS) -Fo$*.obj -Fd$(OBJDIR)\vc140.pdb $<

{./tools}.cpp{$(OBJDIR)}.obj:
   cl $(CPPFLAGS) -Fo$*.obj -Fd$(OBJDIR)\vc140.pdb $<


all:  $(BINDIR) $(OBJDIR) \
        $(BINDIR)\waveformlib.lib \
        $(BINDIR)\wavecompare.exe \
        $(BINDIR)\waveconvert.exe \
        $(BINDIR)\waveextend.exe \
        $(BINDIR)\waveecho.exe \
        $(BINDIR)\waveeq.exe \
        $(BINDIR)\wavefade.exe \
        $(BINDIR)\wavegate.exe \
        $(BINDIR)\waveinfo.exe \
        $(BINDIR)\wavejoin.exe \
        $(BINDIR)\wavemix.exe \
        $(BINDIR)\wavenormalize.exe \
        $(BINDIR)\waveprint.exe \
        $(BINDIR)\waverate.exe \
        $(BINDIR)\wavestretch.exe \
        $(BINDIR)\wavetremolo.exe \
        $(BINDIR)\wavetrim.exe \
        $(BINDIR)\wavevibrato.exe \
        $(BINDIR)\wavevolume.exe \
        $(BINDIR)\unittest.exe \
        $(BINDIR)\ffmpeg.exe

# If a copy of ffmpeg.exe is in a local directory named ffmpeg,
# then copy it to the directory where our compiled binaries
# go. 
$(BINDIR)\ffmpeg.exe: ffmpeg\ffmpeg.exe
    if exist "$**" copy /Y "$**" $@

# Create the subdirectory where the binaries get placed during the build.
$(BINDIR):
    if not exist x64 mkdir x64
    if not exist $(BINDIR) mkdir $(BINDIR)

# Create the subdirectory where the object files get placed during the build.
$(OBJDIR):
    if not exist x64 mkdir x64
    if not exist $(OBJDIR) mkdir $(OBJDIR)

# Create the waveform utility library that all of the tools get linked to.
$(BINDIR)\waveformlib.lib: \
        $(OBJDIR)\waveform.obj \
        $(OBJDIR)\wavfile.obj \
        $(OBJDIR)\wavfile_test.obj \
        $(OBJDIR)\waveformload.obj \
        $(OBJDIR)\rawpcmfile.obj \
        $(OBJDIR)\waveformsave.obj
    lib /NOLOGO /OUT:$@ $**

$(BINDIR)\wavecompare.exe: $(OBJDIR)\wavecompare.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\waveconvert.exe: $(OBJDIR)\waveconvert.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\waveextend.exe: $(OBJDIR)\waveextend.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\waveecho.exe: $(OBJDIR)\waveecho.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\waveeq.exe: $(OBJDIR)\waveeq.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\wavefade.exe: $(OBJDIR)\wavefade.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\wavegate.exe: $(OBJDIR)\wavegate.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\waveinfo.exe: $(OBJDIR)\waveinfo.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\wavejoin.exe: $(OBJDIR)\wavejoin.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\wavemix.exe: $(OBJDIR)\wavemix.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\wavenormalize.exe: $(OBJDIR)\wavenormalize.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\waveprint.exe: $(OBJDIR)\waveprint.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\waverate.exe: $(OBJDIR)\waverate.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\wavestretch.exe: $(OBJDIR)\wavestretch.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\wavetremolo.exe: $(OBJDIR)\wavetremolo.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\wavetrim.exe: $(OBJDIR)\wavetrim.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\wavevibrato.exe: $(OBJDIR)\wavevibrato.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\wavevolume.exe: $(OBJDIR)\wavevolume.obj \
        $(BINDIR)\waveformlib.lib
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

$(BINDIR)\unittest.exe: $(OBJDIR)\unittest.obj \
        $(BINDIR)\waveformlib.lib \
        $(OBJDIR)\wavfile_test.obj \
        $(OBJDIR)\normalize_test.obj \
        $(OBJDIR)\waveformload_test.obj
    link /NOLOGO /DEBUG $** gdi32.lib user32.lib /OUT:$@

#
# Object files for waveform utility tool programs.
#
$(OBJDIR)\wavecompare.obj:     tools/wavecompare.cpp         $(HDRS)
$(OBJDIR)\waveconvert.obj:     tools/waveconvert.cpp         $(HDRS)
$(OBJDIR)\waveextend.obj:      tools/waveextend.cpp          $(HDRS)
$(OBJDIR)\waveecho.obj:        tools/waveecho.cpp            $(HDRS)
$(OBJDIR)\waveeq.obj:          tools/waveeq.cpp              $(HDRS)
$(OBJDIR)\wavefade.obj:        tools/wavefade.cpp            $(HDRS)
$(OBJDIR)\wavegate.obj:        tools/wavegate.cpp            $(HDRS)
$(OBJDIR)\waveinfo.obj:        tools/waveinfo.cpp            $(HDRS)
$(OBJDIR)\wavejoin.obj:        tools/wavejoin.cpp            $(HDRS)
$(OBJDIR)\wavemix.obj:         tools/wavemix.cpp             $(HDRS)
$(OBJDIR)\wavenormalize.obj:   tools/wavenormalize.cpp       $(HDRS)
$(OBJDIR)\waveprint.obj:       tools/waveprint.cpp           $(HDRS)
$(OBJDIR)\waverate.obj:        tools/waverate.cpp            $(HDRS)
$(OBJDIR)\wavestretch.obj:     tools/wavestretch.cpp         $(HDRS)
$(OBJDIR)\wavetremolo.obj:     tools/wavetremolo.cpp         $(HDRS)
$(OBJDIR)\wavetrim.obj:        tools/wavetrim.cpp            $(HDRS)
$(OBJDIR)\wavevibrato.obj:     tools/wavevibrato.cpp         $(HDRS)
$(OBJDIR)\wavevolume.obj:      tools/wavevolume.cpp          $(HDRS)

#
# Object files for waveformlib.
#
$(OBJDIR)\waveform.obj:        libsrc/waveform.cpp           $(HDRS)
$(OBJDIR)\waveformload.obj:    libsrc/waveformload.cpp       $(HDRS)
$(OBJDIR)\waveformsave.obj:    libsrc/waveformsave.cpp       $(HDRS)
$(OBJDIR)\rawpcmfile.obj:      subsys/rawpcmfile.cpp         $(HDRS)
$(OBJDIR)\wavfile.obj:         subsys/wavfile.cpp            $(HDRS)

#
# Object files for unit tests.
#
$(OBJDIR)\unittest.obj:             test/unittest.cpp            $(HDRS)
$(OBJDIR)\normalize_test.obj:       test/normalize_test.cpp      $(HDRS)
$(OBJDIR)\wavfile_test.obj:         test/wavfile_test.cpp        $(HDRS)
$(OBJDIR)\waveformload_test.obj:    test/waveformload_test.cpp   $(HDRS)
# TODO: Implement waveformsave_test

#
# Purge all target and object files, leaving just the source files.
#
clean:
    if exist $(OBJDIR)\*.obj del $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.res del $(OBJDIR)\*.res
    if exist $(OBJDIR)\*.pdb del $(OBJDIR)\*.pdb
    if exist $(BINDIR)\*.exe del $(BINDIR)\*.exe
    if exist $(BINDIR)\*.ilk del $(BINDIR)\*.ilk
    if exist $(BINDIR)\*.pdb del $(BINDIR)\*.pdb
    if exist $(BINDIR)\*.map del $(BINDIR)\*.map
    if exist $(BINDIR)\.vs rmdir /s /q $(BINDIR)\.vs
    if exist $(OBJDIR) rmdir /s /q $(OBJDIR)
    if exist $(BINDIR) rmdir /s /q $(BINDIR)
    if exist x64 rmdir /s /q x64
    if exist .vs rmdir /s /q .vs
    if exist *.user del *.user
    if exist *.bak del *.bak
    if exist *.out del *.out
    if exist testout_* del testout_*
    if exist test\*.out del test\*.out
    if exist test\testout_* del test\testout_*

