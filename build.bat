@echo off
REM Build script for AMD Global Frame Generator
REM Requires: Visual Studio 2019/2022, CMake, and optional vcpkg

setlocal enabledelayedexpansion

echo.
echo ========================================
echo AMD Global Frame Generator - Build Script
echo ========================================
echo.

REM Check if CMake is available
where cmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: CMake not found. Please install CMake and add it to PATH.
    exit /b 1
)

REM Create build directory
if not exist "build" (
    echo Creating build directory...
    mkdir build
)
cd build

REM Configure with CMake
echo.
echo Configuring project with CMake...
echo.

if "%VCPKG_ROOT%"=="" (
    REM Build without vcpkg (will try to find MinHook in system)
    cmake -G "Visual Studio 17 2022" -A x64 ..
) else (
    REM Build with vcpkg integration
    cmake -G "Visual Studio 17 2022" -A x64 ^
        -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ..
)

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed!
    echo Please ensure you have:
    echo - Visual Studio 2019 or 2022 installed
    echo - CMake 3.20 or later
    echo - DirectX 12 SDK
    exit /b 1
)

REM Build the project
echo.
echo Building project...
echo.

cmake --build . --config Release

if errorlevel 1 (
    echo.
    echo ERROR: Build failed!
    cd ..
    exit /b 1
)

REM Create output directories
if not exist "bin" mkdir bin

REM Copy binaries to bin directory
echo.
echo Copying binaries to bin directory...

if exist "Release\afglobal.exe" (
    copy /Y "Release\afglobal.exe" "bin\"
    echo Copied afglobal.exe
)

if exist "Release\afglobal_hook.dll" (
    copy /Y "Release\afglobal_hook.dll" "bin\"
    echo Copied afglobal_hook.dll
)

if exist "..\afglobal_config.json" (
    copy /Y "..\afglobal_config.json" "bin\"
    echo Copied afglobal_config.json
)

cd ..

echo.
echo ========================================
echo Build Complete!
echo ========================================
echo.
echo Output files located in: build\bin\
echo   - afglobal.exe (Main executable)
echo   - afglobal_hook.dll (Injection DLL)
echo   - afglobal_config.json (Configuration)
echo.
echo Next steps:
echo 1. Test the application: build\bin\afglobal.exe
echo 2. Edit configuration: build\bin\afglobal_config.json
echo 3. Create installer: iscc installer\afglobal_installer.iss
echo.
echo For manual installation:
echo   Copy build\bin\* to C:\Program Files\AMDGlobalFG
echo.
pause
