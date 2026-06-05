@echo off
setlocal
cd /d %~dp0\..
build\Release\pqc_node.exe status
