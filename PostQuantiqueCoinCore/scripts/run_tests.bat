@echo off
setlocal
cd /d "%~dp0\..\build"
ctest -C Release --output-on-failure
exit /b %errorlevel%
