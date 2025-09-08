@echo off

    set TLOG=wavecomparetest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\wavecompare.exe
    if not exist %TEXE% goto exe_missing
    set TCVT=..\x64\Release\waveconvert.exe
    if not exist %TCVT% goto cvt_missing

    echo Running tests with "%TEXE%".  One moment...
    echo ===================================                      >> %TLOG%

    rem ###
    rem ### Compare a file to itself, which should be identical.
    rem ###
    %TEXE% ..\testdata\blue.mp3 ..\testdata\blue.mp3  >> %TLOG%
    if errorlevel 1 goto compare_failed
    echo ===================================                      >> %TLOG%

    rem ###
    rem ### Compare another file to itself, which should be identical.
    rem ###
    %TEXE% ..\testdata\airhost.wav ..\testdata\airhost.wav        >> %TLOG%
    if errorlevel 1 goto compare_failed
    echo ===================================                      >> %TLOG%

    rem ###
    rem ### Convert an MP3 to WAV and compare the two.
    rem ###
    %TCVT% ..\testdata\blue.mp3 testout_blue.wav      >> %TLOG%
    if errorlevel 1 goto test_failed
    %TEXE% ..\testdata\blue.mp3 testout_blue.wav      >> %TLOG%
    if errorlevel 1 goto compare_failed
    echo ===================================                      >> %TLOG%

    rem ###
    rem ### Convert an MP3 to WAV and compare the two.
    rem ###
    %TCVT% ..\testdata\strum.mp3 testout_strum.wav  >> %TLOG%
    if errorlevel 1 goto test_failed
    %TEXE% ..\testdata\strum.mp3 testout_strum.wav  >> %TLOG%
    if errorlevel 1 goto compare_failed
    echo ===================================                      >> %TLOG%

    rem ###
    rem ### Compare two totally different files.
    rem ### This comparison should fail.
    rem ###
    echo Comparing files that are different.  This compare should fail. >> %TLOG%
    %TEXE% ..\testdata\blue.mp3 ..\testdata\strum.mp3  >> %TLOG%
    if errorlevel 1 goto compare_ok
    goto compare_failed
:compare_ok
    echo ===================================                      >> %TLOG%


:skip
    echo Done running tests. >> %TLOG%
    echo Done running tests.  See %TLOG% for test results.
    set TEXE=
    set TLOG=
    set TCVT=
    exit /b 0


:test_failed
    echo ERROR:  Test failed. >> %TLOG%
    echo ERROR:  Test failed.  See %TLOG% for test results.
    set TEXE=
    set TLOG=
    set TCVT=
    exit /b 1


:compare_failed
    echo ERROR:  Comparison result didn't match expected result. >> %TLOG%
    echo ERROR:  Comparison result didn't match expected result.
    echo         See %TLOG% for test results.
    set TEXE=
    set TLOG=
    set TCVT=
    exit /b 1


:exe_missing
    echo ERROR:  Program "%TEXE%" doesn't exist.  Has it been compiled yet? >> %TLOG%
    echo ERROR:  Program "%TEXE%" doesn't exist.  Has it been compiled yet?
    set TEXE=
    set TLOG=
    set TCVT=
    exit /b 1


:cvt_missing
    echo ERROR:  Program "%TCVT%" doesn't exist.  Has it been compiled yet? >> %TLOG%
    echo ERROR:  Program "%TCVT%" doesn't exist.  Has it been compiled yet?
    set TEXE=
    set TLOG=
    set TCVT=
    exit /b 1

