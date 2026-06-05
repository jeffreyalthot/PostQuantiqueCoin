@echo off
setlocal
cd /d %~dp0\..
rmdir /s /q build-debug 2>nul
rmdir /s /q build-release 2>nul
call scripts\build_debug.bat
if errorlevel 1 exit /b 1
call scripts\build_release.bat
if errorlevel 1 exit /b 1
call scripts\run_all_tests.bat
