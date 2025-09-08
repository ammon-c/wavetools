@echo off

    set TLOG=waveconverttest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\waveconvert.exe
    if not exist %TEXE% goto exe_missing

    echo Running tests with "%TEXE%".  One moment...
    echo ===================================                  >> %TLOG%

    %TEXE% ..\testdata\airhost.wav testout_airhost.raw        >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%
    %TEXE% ..\testdata\airhost.wav testout_airhost.wav        >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%
    %TEXE% -Mono ..\testdata\airhost.wav testout_airhost_mono.wav  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%
    %TEXE% -Stereo ..\testdata\airhost.wav testout_airhost_stereo.wav  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%
    %TEXE% ..\testdata\airhost.wav testout_airhost.mp3        >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%

    %TEXE% ..\testdata\testing123.wav testout_testing123.raw        >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%
    %TEXE% ..\testdata\testing123.wav testout_testing123.wav        >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%
    %TEXE% ..\testdata\testing123.wav testout_testing123.mp3        >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%

    %TEXE% ..\testdata\counting.wav testout_counting.raw      >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%
    %TEXE% ..\testdata\counting.wav testout_counting.wav      >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%
    %TEXE% ..\testdata\counting.wav testout_counting.mp3      >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%

    %TEXE% ..\testdata\blue.mp3 testout_blue.raw  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%
    %TEXE% ..\testdata\blue.mp3 testout_blue.wav  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%
    %TEXE% -Mono ..\testdata\blue.mp3 testout_blue_mono.wav  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%
    %TEXE% ..\testdata\blue.mp3 testout_blue.mp3  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%
    %TEXE% -Mono ..\testdata\blue.mp3 testout_blue_mono.mp3  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%

    %TEXE% ..\testdata\strum.mp3 testout_strum.raw  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%
    %TEXE% ..\testdata\strum.mp3 testout_strum.wav  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%
    %TEXE% ..\testdata\strum.mp3 testout_strum.mp3  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                  >> %TLOG%


:skip
    echo Done running tests. >> %TLOG%
    echo Done running tests.  See %TLOG% for test results.
    set TEXE=
    set TLOG=
    exit /b 0


:test_failed
    echo ERROR:  Test failed. >> %TLOG%
    echo ERROR:  Test failed.  See %TLOG% for test results.
    set TEXE=
    set TLOG=
    exit /b 1


:exe_missing
    echo ERROR:  Program "%TEXE%" doesn't exist.  Has it been compiled yet? >> %TLOG%
    echo ERROR:  Program "%TEXE%" doesn't exist.  Has it been compiled yet?
    set TEXE=
    set TLOG=
    exit /b 1

