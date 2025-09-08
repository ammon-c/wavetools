@echo off

    set TLOG=waveratetest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\waverate.exe
    if not exist %TEXE% goto exe_missing

    echo Running tests with "%TEXE%".  One moment...
    echo ===================================                              >> %TLOG%

    %TEXE% 8000 ..\testdata\airhost.wav testout_airhost.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 22500 ..\testdata\airhost.wav testout_airhost_22k.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 44100 ..\testdata\airhost.wav testout_airhost_44k.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% 8000 ..\testdata\testing123.wav testout_testing123.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 22500 ..\testdata\testing123.wav testout_testing123_22k.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 44100 ..\testdata\testing123.wav testout_testing123_44k.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% 8000 ..\testdata\counting.wav testout_counting.wav       >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 22500 ..\testdata\counting.wav testout_counting_22k.wav  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 44100 ..\testdata\counting.wav testout_counting_44k.wav  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% 8000 ..\testdata\blue.mp3 testout_blue.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 22500 ..\testdata\blue.mp3 testout_blue_22k.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 44100 ..\testdata\blue.mp3 testout_blue_44k.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%

    %TEXE% 8000  ..\testdata\strum.mp3 testout_strum.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 22500 ..\testdata\strum.mp3 testout_strum_22k.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 44100 ..\testdata\strum.mp3 testout_strum_44k.wav   >> %TLOG%
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

