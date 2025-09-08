@echo off

    set TLOG=wavemixtest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\wavemix.exe
    if not exist %TEXE% goto exe_missing

    echo Running tests with "%TEXE%".  One moment...
    echo ===================================                               >> %TLOG%

    %TEXE% testout_mix1.wav ..\testdata\airhost.wav ..\testdata\chug.mp3   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%

    %TEXE% testout_mix2.wav ..\testdata\airhost.wav,0.5,1.0 ..\testdata\airhost.wav,0.5,2.0 ..\testdata\chug.mp3,0.3,5.0   >> %TLOG%
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

