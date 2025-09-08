@echo off

    set TLOG=waveechotest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\waveecho.exe
    if not exist %TEXE% goto exe_missing

    echo Running tests with "%TEXE%".  One moment...
    echo ===================================                              >> %TLOG%

    %TEXE% 500 3 ..\testdata\airhost.wav testout_airhost_echo1.wav      >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 50 1 ..\testdata\airhost.wav testout_airhost_echo2.wav     >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 1000 5 -WetLevel=0.7 ..\testdata\airhost.wav testout_airhost_echo3.wav     >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% 500 3 ..\testdata\testing123.wav testout_testing123_echo1.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 50 1 ..\testdata\testing123.wav testout_testing123_echo2.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 1000 5 -WetLevel=0.7 ..\testdata\testing123.wav testout_testing123_echo3.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% 500 3 ..\testdata\blue.mp3 testout_blue_echo1.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 50 1 ..\testdata\blue.mp3 testout_blue_echo2.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 1000 5 -WetLevel=0.7 ..\testdata\blue.mp3 testout_blue_echo3.wav    >> %TLOG%
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

