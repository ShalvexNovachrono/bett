@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 -vcvars_ver=14.44
cmake --preset debug
if exist "%~dp0build\compile_commands.json" (
    copy /Y "%~dp0build\compile_commands.json" "%~dp0compile_commands.json" >nul
)
