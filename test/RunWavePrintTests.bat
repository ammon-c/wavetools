@echo off

    set TLOG=waveprinttest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\waveprint.exe
    if not exist %TEXE% goto exe_missing

    echo Running tests with "%TEXE%".  One moment...
    echo ===================================    >> %TLOG%
    %TEXE% ..\testdata\airhost.wav              >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================    >> %TLOG%
    %TEXE% -Min=-0.2 -Max=0.2 ..\testdata\testing123.wav     >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================    >> %TLOG%
    %TEXE% ..\testdata\counting.wav             >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================    >> %TLOG%

    %TEXE% -PerLine=40000 -Width=65 ..\testdata\blue.mp3           >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================    >> %TLOG%
    %TEXE% -PerLine=40000 -Width=65 ..\testdata\strum.mp3     >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================    >> %TLOG%


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

