@echo off

    set TLOG=waveextendtest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\waveextend.exe
    if not exist %TEXE% goto exe_missing

    echo Running tests with "%TEXE%".  One moment...
    echo ===================================                              >> %TLOG%

    %TEXE% 0 0 ..\testdata\airhost.wav testout_airhost_x1.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 250000 0 ..\testdata\airhost.wav testout_airhost_x2.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 0 260000 ..\testdata\airhost.wav testout_airhost_x3.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 250000 260000 ..\testdata\airhost.wav testout_airhost_x4.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -UseTime 0 0 ..\testdata\airhost.wav testout_airhost_x5.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -UseTime 2.5 0 ..\testdata\airhost.wav testout_airhost_x6.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -UseTime 0 3.5 ..\testdata\airhost.wav testout_airhost_x7.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -UseTime 2.5 3.5 ..\testdata\airhost.wav testout_airhost_x8.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% 0 0 ..\testdata\blue.mp3 testout_blue_x1.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 300000 0 ..\testdata\blue.mp3 testout_blue_x2.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 0 350000 ..\testdata\blue.mp3 testout_blue_x3.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 300000 350000 ..\testdata\blue.mp3 testout_blue_x4.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -UseTime 0 0 ..\testdata\blue.mp3 testout_blue_x5.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -UseTime 2.5 0 ..\testdata\blue.mp3 testout_blue_x6.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -UseTime 0 3.5 ..\testdata\blue.mp3 testout_blue_x7.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -UseTime 2.5 3.5 ..\testdata\blue.mp3 testout_blue_x8.wav    >> %TLOG%
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

