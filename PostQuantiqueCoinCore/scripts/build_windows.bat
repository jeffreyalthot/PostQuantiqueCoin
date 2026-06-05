@echo off
setlocal
cd /d "%~dp0\.."
if not exist build mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
if errorlevel 1 exit /b 1
cmake --build . --config Release
exit /b %errorlevel%
