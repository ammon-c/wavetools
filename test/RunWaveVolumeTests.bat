@echo off

    set TLOG=wavevolumetest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\wavevolume.exe
    if not exist %TEXE% goto exe_missing

    echo Running tests with "%TEXE%".  One moment...
    echo ===================================                              >> %TLOG%

    %TEXE% 1.0 ..\testdata\airhost.wav testout_airhost_v1.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 0.5 ..\testdata\airhost.wav testout_airhost_vlo.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 2.0 ..\testdata\airhost.wav testout_airhost_vhi.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% 1.0 ..\testdata\testing123.wav testout_testing123_v1.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 0.5 ..\testdata\testing123.wav testout_testing123_vlo.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 2.0 ..\testdata\testing123.wav testout_testing123_vhi.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% 1.0 ..\testdata\blue.mp3 testout_blue_v1.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 0.5 ..\testdata\blue.mp3 testout_blue_vlo.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 2.0 ..\testdata\blue.mp3 testout_blue_vhi.wav    >> %TLOG%
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

