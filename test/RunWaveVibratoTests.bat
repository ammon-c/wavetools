@echo off

    set TLOG=wavevibratotest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\wavevibrato.exe
    if not exist %TEXE% goto exe_missing

    echo Running tests with "%TEXE%".  One moment...
    echo ===================================                              >> %TLOG%

    %TEXE% 0.5 5 -WetLevel=0.5 -DryLevel=0.5 ..\testdata\airhost.wav testout_airhost_vib1.wav      >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 2.0 50 ..\testdata\airhost.wav testout_airhost_vib2.wav     >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 0.2 7 ..\testdata\airhost.wav testout_airhost_vib3.wav     >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 10 5 -WetLevel=0.5 -DryLevel=0.5 ..\testdata\airhost.wav testout_airhost_vib4.wav     >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% 0.5 5 -WetLevel=0.5 -DryLevel=0.5 ..\testdata\testing123.wav testout_testing123_vib1.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 2.0 50 ..\testdata\testing123.wav testout_testing123_vib2.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 0.2 7 ..\testdata\testing123.wav testout_testing123_vib3.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 10 5 -WetLevel=0.5 -DryLevel=0.5 ..\testdata\testing123.wav testout_testing123_vib4.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% 0.5 5 -WetLevel=0.5 -DryLevel=0.5 ..\testdata\blue.mp3 testout_blue_vib1.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 2.0 50 ..\testdata\blue.mp3 testout_blue_vib2.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 0.2 7 ..\testdata\blue.mp3 testout_blue_vib3.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 10 5 -WetLevel=0.5 -DryLevel=0.5 ..\testdata\blue.mp3 testout_blue_vib4.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%


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

