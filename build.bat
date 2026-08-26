@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 -vcvars_ver=14.44
if not exist "%~dp0build\CMakeCache.txt" (
    cmake --preset debug
)
cmake --build build --config Debug
