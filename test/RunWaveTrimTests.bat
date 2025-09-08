@echo off

    set TLOG=wavetrimtest.out
    if exist %TLOG% del %TLOG%
    set TEXE=..\x64\Release\wavetrim.exe
    if not exist %TEXE% goto exe_missing

    echo Running tests with "%TEXE%".  One moment...
    echo ===================================                              >> %TLOG%

    %TEXE% -Start=0      -Count=20000 ..\testdata\airhost.wav testout_airhost_t1.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -Start=100000 -Count=20000 ..\testdata\airhost.wav testout_airhost_t2.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -Start=END    -Count=150000 ..\testdata\airhost.wav testout_airhost_t3.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% -Invert -Start=0      -Count=20000 ..\testdata\airhost.wav testout_airhost_ti1.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -Invert -Start=100000 -Count=20000 ..\testdata\airhost.wav testout_airhost_ti2.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -Invert -Start=END    -Count=150000 ..\testdata\airhost.wav testout_airhost_ti3.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% -UseTime -Start=0      -Count=1.5 ..\testdata\airhost.wav testout_airhost_t4.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -UseTime -Start=1.5    -Count=1.5 ..\testdata\airhost.wav testout_airhost_t5.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%
    %TEXE% -UseTime -Start=END    -Count=3.25 ..\testdata\airhost.wav testout_airhost_t6.wav   >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                              >> %TLOG%

    %TEXE% -Start=0      -Count=20000 ..\testdata\blue.mp3 testout_blue_t1.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -Start=100000 -Count=20000 ..\testdata\blue.mp3 testout_blue_t2.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -Start=END    -Count=150000 ..\testdata\blue.mp3 testout_blue_t3.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%

    %TEXE% -Invert -Start=0      -Count=20000 ..\testdata\blue.mp3 testout_blue_ti1.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -Invert -Start=100000 -Count=20000 ..\testdata\blue.mp3 testout_blue_ti2.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -Invert -Start=END    -Count=150000 ..\testdata\blue.mp3 testout_blue_ti3.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%

    %TEXE% -UseTime -Start=0      -Count=1.5 ..\testdata\blue.mp3 testout_blue_t4.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -UseTime -Start=1.5    -Count=1.5 ..\testdata\blue.mp3 testout_blue_t5.wav    >> %TLOG%
    if errorlevel 1 goto test_failed
    echo ===================================                               >> %TLOG%
    %TEXE% -UseTime -Start=END    -Count=5.25 ..\testdata\blue.mp3 testout_blue_t6.wav    >> %TLOG%
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

