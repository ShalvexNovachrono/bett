@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 -vcvars_ver=14.44
echo Generating compile_commands.json...
cmake --preset debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
if exist "%~dp0build\compile_commands.json" (
    copy /Y "%~dp0build\compile_commands.json" "%~dp0compile_commands.json" >nul
    echo [SUCCESS] compile_commands.json generated and copied to project root.
) else (
    echo [ERROR] Could not find build\compile_commands.json
)
