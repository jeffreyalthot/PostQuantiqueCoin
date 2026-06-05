@echo off
setlocal
cd /d %~dp0\..
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1
cmake --build build-release --config Release
