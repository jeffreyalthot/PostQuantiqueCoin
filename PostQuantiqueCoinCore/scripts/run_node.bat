@echo off
setlocal
cd /d "%~dp0\..\build"
if exist Release\pqc_node.exe (
  Release\pqc_node.exe %*
) else (
  echo pqc_node.exe is not built by the current first-delivery target set.
  exit /b 2
)
