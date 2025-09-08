@echo off

    set TLOG=wavejointest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\wavejoin.exe
    if not exist %TEXE% goto exe_missing

    echo Running tests with "%TEXE%".  One moment...
    echo =================================== >> %TLOG%

    %TEXE% ..\testdata\airhost.wav ..\testdata\testing123.wav testout_join1.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo =================================== >> %TLOG%

    %TEXE% ..\testdata\counting.wav ..\testdata\strum.mp3 ..\testdata\airhost.wav testout_join2.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo =================================== >> %TLOG%

    %TEXE% ..\testdata\blue.mp3 ..\testdata\strum.mp3 testout_join3.mp3   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo =================================== >> %TLOG%

rem DEBUG!!!

    %TEXE% ..\testdata\blue.mp3 testout_join4.mp3   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo =================================== >> %TLOG%

    %TEXE% ..\testdata\strum.mp3 testout_join5.mp3   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo =================================== >> %TLOG%


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

