@echo off

    set TLOG=wavenormalizetest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\wavenormalize.exe
    if not exist %TEXE% goto exe_missing

    echo Running tests with "%TEXE%".  One moment...
    echo ===================================                              >> %TLOG%

    %TEXE% 0 ..\testdata\airhost.wav testout_airhost_n0.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -1.0 ..\testdata\airhost.wav testout_airhost_n1.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -5.5 ..\testdata\airhost.wav testout_airhost_n2.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% 0 ..\testdata\testing123.wav testout_testing123_n0.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -1.0 ..\testdata\testing123.wav testout_testing123_n1.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -5.5 ..\testdata\testing123.wav testout_testing123_n2.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% 0 ..\testdata\blue.mp3 testout_blue_n0.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -1.0 ..\testdata\blue.mp3 testout_blue_n1.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -5.5 ..\testdata\blue.mp3 testout_blue_n2.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%

    %TEXE% 0 ..\testdata\counting.wav testout_counting_n0.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -1.0 ..\testdata\counting.wav testout_counting_n1.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -5.5 ..\testdata\counting.wav testout_counting_n2.wav    >> %TLOG%
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

