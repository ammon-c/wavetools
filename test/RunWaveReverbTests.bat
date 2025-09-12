@echo off

    set TLOG=wavereverbtest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\wavereverb.exe
    if not exist %TEXE% goto exe_missing

    echo Running tests with "%TEXE%".  One moment...
    echo ===================================                              >> %TLOG%

    %TEXE% 0.1 ..\testdata\airhost.wav testout_airhost_rev1.wav           >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 0.2 ..\testdata\airhost.wav testout_airhost_rev2.wav           >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 0.3 ..\testdata\airhost.wav testout_airhost_rev3.wav           >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 1.0 ..\testdata\airhost.wav testout_airhost_rev4.wav           >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% 0.1 ..\testdata\testing123.wav testout_testing123_rev1.wav     >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 0.2 ..\testdata\testing123.wav testout_testing123_rev2.wav     >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 0.3 ..\testdata\testing123.wav testout_testing123_rev3.wav     >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 1.0 ..\testdata\testing123.wav testout_testing123_rev4.wav     >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% 0.1 ..\testdata\blue.mp3 testout_blue_rev1.wav                 >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 0.2 ..\testdata\blue.mp3 testout_blue_rev2.wav                 >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 0.3 ..\testdata\blue.mp3 testout_blue_rev3.wav                 >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% 1.0 ..\testdata\blue.mp3 testout_blue_rev4.wav                 >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%


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

