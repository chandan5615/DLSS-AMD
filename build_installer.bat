@echo off
REM Inno Setup Installer Build Script
REM Requires: Inno Setup 6.0 or later

setlocal enabledelayedexpansion

echo.
echo ========================================
echo AMD Global Frame Generator - Installer Builder
echo ========================================
echo.

REM Look for Inno Setup installation
set ISCC=""
for /f "tokens=*" %%i in ('where iscc.exe 2^>nul') do set ISCC=%%i

if "%ISCC%"=="" (
    REM Try common installation paths
    if exist "C:\Program Files (x86)\Inno Setup 6\iscc.exe" (
        set ISCC=C:\Program Files (x86)\Inno Setup 6\iscc.exe
    ) else if exist "C:\Program Files\Inno Setup 6\iscc.exe" (
        set ISCC=C:\Program Files\Inno Setup 6\iscc.exe
    )
)

if "%ISCC%"=="" (
    echo ERROR: Inno Setup not found!
    echo Please install Inno Setup 6.0 or later from:
    echo https://jrsoftware.org/isdl.php
    exit /b 1
)

echo Found Inno Setup at: %ISCC%
echo.

REM Verify build artifacts exist
if not exist "build\bin\afglobal.exe" (
    echo ERROR: afglobal.exe not found!
    echo Please run build.bat first to compile the project.
    exit /b 1
)

if not exist "build\bin\afglobal_hook.dll" (
    echo ERROR: afglobal_hook.dll not found!
    echo Please run build.bat first to compile the project.
    exit /b 1
)

echo Build artifacts verified.
echo.

REM Create output directory for installer
if not exist "installer\output" mkdir installer\output

echo Building installer...
echo.

REM Run Inno Setup compiler
"%ISCC%" /O"installer\output" installer\afglobal_installer.iss

if errorlevel 1 (
    echo.
    echo ERROR: Installer build failed!
    exit /b 1
)

echo.
echo ========================================
echo Installer Build Complete!
echo ========================================
echo.
echo Installer created at:
echo   installer\output\AFGlobalFGSetup.exe
echo.
echo To install AMD Global Frame Generator:
echo   1. Run installer\output\AFGlobalFGSetup.exe
echo   2. Follow the installation wizard
echo   3. Optionally enable "Run on Windows Startup"
echo.
pause
