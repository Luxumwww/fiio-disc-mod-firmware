@echo off
setlocal DisableDelayedExpansion
chcp 65001 >nul
title DISC - Flash
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -STA -ExecutionPolicy Bypass -File "%~dp0_内部\操作工具.ps1" -Mode Flash
set "disc_exit=%errorlevel%"
echo.
pause
exit /b %disc_exit%
