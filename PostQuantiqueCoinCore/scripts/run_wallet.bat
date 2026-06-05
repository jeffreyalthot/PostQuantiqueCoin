@echo off
setlocal
cd /d "%~dp0\..\build"
if exist Release\pqc_wallet.exe (
  Release\pqc_wallet.exe %*
) else (
  echo pqc_wallet.exe is not built by the current first-delivery target set.
  exit /b 2
)
