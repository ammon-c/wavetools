@echo off

    set TLOG=waveeqtest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\waveeq.exe
    if not exist %TEXE% goto exe_missing

    echo Running tests with "%TEXE%".  One moment...
    echo ===================================                               >> %TLOG%

    %TEXE% 3000 500 ..\testdata\airhost.wav testout_airhost_eq1.wav        >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 1000 0 ..\testdata\airhost.wav testout_airhost_eq2.wav          >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 0 2000 ..\testdata\airhost.wav testout_airhost_eq3.wav          >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 0 0 -BandpassFreq=3000 -BandpassQ=5.0 ..\testdata\airhost.wav testout_airhost_eq4.wav          >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 0 0 -NotchFreq=3000 -NotchQ=2.0 ..\testdata\airhost.wav testout_airhost_eq5.wav          >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%

    %TEXE% 3000 500 ..\testdata\testing123.wav testout_testing123_eq1.wav        >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 1000 0 ..\testdata\testing123.wav testout_testing123_eq2.wav          >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 0 2000 ..\testdata\testing123.wav testout_testing123_eq3.wav          >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 0 0 -BandpassFreq=3000 -BandpassQ=5.0 ..\testdata\testing123.wav testout_testing123_eq4.wav          >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 0 0 -NotchFreq=3000 -NotchQ=2.0 ..\testdata\testing123.wav testout_testing123_eq5.wav          >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%

    %TEXE% 3000 500 ..\testdata\blue.mp3 testout_blue_eq1.wav              >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 1000 0 ..\testdata\blue.mp3 testout_blue_eq2.wav                >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 0 2000 ..\testdata\blue.mp3 testout_blue_eq3.wav                >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 0 0 -BandpassFreq=3000 -BandpassQ=5.0 ..\testdata\blue.mp3 testout_blue_eq4.wav                >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% 0 0 -NotchFreq=3000 -NotchQ=2.0 ..\testdata\blue.mp3 testout_blue_eq5.wav                >> %TLOG%
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

