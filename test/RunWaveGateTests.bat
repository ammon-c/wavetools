@echo off

    set TLOG=wavegatetest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\wavegate.exe
    if not exist %TEXE% goto exe_missing

    echo Running tests with "%TEXE%".  One moment...
    echo ===================================                              >> %TLOG%

    %TEXE% -TrimStart -TrimEnd               ..\testdata\airhost.wav testout_airhost_g1.wav  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -TrimStart -TrimEnd -Threshold=0.2 ..\testdata\airhost.wav testout_airhost_g2.wav  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -TrimStart -TrimEnd -Threshold=0.4 ..\testdata\airhost.wav testout_airhost_g3.wav  >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% -TrimStart -TrimEnd                ..\testdata\testing123.wav testout_testing123_g1.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -TrimStart -TrimEnd -Threshold=0.2 ..\testdata\testing123.wav testout_testing123_g2.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -TrimStart -TrimEnd -Threshold=0.4 ..\testdata\testing123.wav testout_testing123_g3.wav         >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% -TrimStart -TrimEnd                ..\testdata\blue.mp3 testout_blue_g1.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -TrimStart -TrimEnd -Threshold=0.2 ..\testdata\blue.mp3 testout_blue_g2.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -TrimStart -TrimEnd -Threshold=0.4 ..\testdata\blue.mp3 testout_blue_g3.wav    >> %TLOG%
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

