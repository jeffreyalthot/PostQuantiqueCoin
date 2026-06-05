@echo off
setlocal
cd /d %~dp0\..
if not exist build-debug cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
if errorlevel 1 exit /b 1
cmake --build build-debug --config Debug --target pqc_tests
if errorlevel 1 exit /b 1
ctest --test-dir build-debug -C Debug --output-on-failure
if errorlevel 1 exit /b 1
build-debug\Debug\pqc_node.exe status
build-debug\Debug\pqc_wallet.exe help
build-debug\Debug\pqc_miner.exe benchmark
