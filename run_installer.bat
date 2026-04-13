@echo off
REM AMD Global Frame Generator - Installer Launcher
REM This batch file bypasses some policy restrictions

cd /d "%~dp0"
echo Starting AMD Global Frame Generator Installer...
start "" "installer\output\AFGlobalFGSetup.exe"
