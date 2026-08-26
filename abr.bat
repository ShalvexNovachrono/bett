@echo off
call build.bat
if %ERRORLEVEL% NEQ 0 (
    echo Build failed with error level %ERRORLEVEL%.
    exit /b %ERRORLEVEL%
)
REM Clearing the console before running
cls
call run.bat %*
