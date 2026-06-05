@echo off
setlocal
cd /d %~dp0\..
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
if errorlevel 1 exit /b 1
cmake --build build-debug --config Debug
