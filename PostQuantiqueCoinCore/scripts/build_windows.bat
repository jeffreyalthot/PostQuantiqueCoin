@echo off
setlocal
cd /d %~dp0\..
if not exist build mkdir build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DUSE_LIBOQS=OFF -DDEV_ONLY_ALLOW_INSECURE_CRYPTO=ON
if errorlevel 1 exit /b 1
cmake --build build --config Release
if errorlevel 1 exit /b 1
echo Executables:
echo %cd%\build\Release\pqc_tests.exe
echo %cd%\build\Release\pqc_node.exe
echo %cd%\build\Release\pqc_wallet.exe
echo %cd%\build\Release\pqc_miner.exe
