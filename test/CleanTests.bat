@echo off

    echo Removing test output files.
    if exist *.out del *.out
    if exist testout_* del testout_*
    echo Done.

