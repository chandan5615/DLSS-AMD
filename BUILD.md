# Building from Source

Complete guide to building AMD Global Frame Generator from source.

## Prerequisites

### System Requirements
- Windows 10 or Windows 11 (x64 only)
- Administrator privileges (required for build and installation)
- At least 5 GB free disk space

### Required Software

1. **Visual Studio 2019 or 2022**
   - Community Edition is sufficient
   - Install: Desktop Development with C++
   - Required workload: C++ desktop development
   - Optional: MSVC v143 toolset

2. **CMake 3.20 or later**
   - Download from: https://cmake.org/download/
   - Add to PATH during installation
   - Test: Open cmd, type `cmake --version`

3. **Git** (optional, for dependencies)
   - Download from: https://git-scm.com/

### Optional Software

- **vcpkg** - For easier dependency management
  ```bash
  git clone https://github.com/Microsoft/vcpkg.git
  cd vcpkg
  .\vcpkg\bootstrap-vcpkg.bat
  ```

- **Inno Setup 6.0 or later** - To build installer
  - Download from: https://jrsoftware.org/isdl.php

## Setup Instructions

### Step 1: Clone Repository

```bash
git clone https://github.com/yourname/AFGlobalFrameGenerator.git
cd AFGlobalFrameGenerator
```

Or download and extract the source zip file.

### Step 2: Install Dependencies

#### Option A: Using vcpkg (Recommended)

```bash
# In vcpkg directory
vcpkg integrate install

# Install MinHook
vcpkg install minhook:x64-windows

# Install nlohmann/json
vcpkg install nlohmann-json:x64-windows
```

#### Option B: Manual Setup

1. **MinHook 3.3.2**
   - Download: https://github.com/TsudaKageyu/minhook/releases/tag/v3.3.2
   - Extract to: `C:\libs\minhook-3.3.2\`
   - Required files:
     - `include/minhook.h`
     - `lib/minhook.lib` (for x64)

2. **nlohmann/json**
   - Download: https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
   - Save to: `C:\libs\nlohmann\json.hpp`

### Step 3: Setup Environment Variables (Optional)

For manual setup, add to your environment:

```batch
set MINHOOK_ROOT=C:\libs\minhook-3.3.2
set JSON_ROOT=C:\libs\nlohmann
```

## Building

### Option 1: Using build.bat (Easiest)

```bash
cd AFGlobalFrameGenerator
build.bat
```

This script:
1. Creates `build/` directory
2. Runs CMake configuration
3. Compiles all targets
4. Copies binaries to `build/bin/`

### Option 2: Using build.ps1 (PowerShell)

```powershell
cd AFGlobalFrameGenerator
.\build.ps1

# With vcpkg:
.\build.ps1 -UseVcpkg -VcpkgRoot "C:\path\to\vcpkg"
```

### Option 3: Manual CMake Build

```bash
mkdir build
cd build

# Configure (without vcpkg)
cmake -G "Visual Studio 17 2022" -A x64 ..

# Configure (with vcpkg)
cmake -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_TOOLCHAIN_FILE=path\to\vcpkg\scripts\buildsystems\vcpkg.cmake ^
      ..

# Build
cmake --build . --config Release

# Output: Release/afglobal.exe and Release/afglobal_hook.dll
```

### Option 4: Using Visual Studio IDE

1. Open Visual Studio
2. File → Open → Folder
3. Select `AFGlobalFrameGenerator` folder
4. CMake configuration runs automatically
5. Build → Build All
6. Output in: `build\Release\`

## Build Troubleshooting

### CMake Not Found

**Error**: `'cmake' is not recognized as an internal or external command`

**Solution**:
1. Install CMake from https://cmake.org/download/
2. Check "Add CMake to system PATH" during installation
3. Restart command prompt

### MinHook Not Found

**Error**: `Could not find MinHook`

**Solution**:
```bash
# Install via vcpkg:
vcpkg install minhook:x64-windows

# Or set environment variable:
set MINHOOK_ROOT=C:\libs\minhook-3.3.2
```

### JSON Header Not Found

**Error**: `nlohmann/json.hpp not found`

**Solution**:
```bash
# Install via vcpkg:
vcpkg install nlohmann-json:x64-windows

# Or manual setup:
# Create: C:\libs\nlohmann\json.hpp
```

### DirectX SDK Not Found

**Error**: `DXGI.lib not found` or `d3d12.lib not found`

**Solution**:
- These are included with Windows SDK
- Should be auto-found by Visual Studio
- If not, install Windows SDK:
  - Run Visual Studio Installer
  - Modify installation
  - Add: Windows 10/11 SDK

### Compilation Errors

**Common Issues**:

1. **C++17 Not Enabled**
   ```
   Error: 'variant' not found in namespace std
   ```
   - Solution: Add `-std:c++17` to compiler flags
   - Already set in CMakeLists.txt

2. **Different Runtime Library**
   ```
   Error: LNK2038: mismatch detected for 'RuntimeLibrary'
   ```
   - Solution: Ensure all libraries use same runtime
   - CMakeLists.txt sets: `MSVC_RUNTIME_LIBRARY`

3. **Missing #include**
   ```
   Error: 'undefined reference to ...'
   ```
   - Solution: Check that all .cpp files are in `set(SOURCES ...)`
   - Verify include paths

## Testing the Build

### 1. Check Output Files

```bash
# After build, verify outputs exist:
dir build\bin\

# Required files:
# - afglobal.exe (2-4 MB)
# - afglobal_hook.dll (1-2 MB)
# - afglobal_config.json
```

### 2. Test Executable

```bash
# Start application (runs in system tray)
build\bin\afglobal.exe

# Check for tray icon in notification area
# Right-click to test menu
# Press Ctrl+Shift+F to test hotkey
```

### 3. Check Logs

```bash
# View application logs
notepad build\bin\afglobal.log

# Should show:
# [timestamp] [INFO] === AMD Global Frame Generator Started ===
# [timestamp] [INFO] Application initialized successfully
```

### 4. Launch a Game

1. Run `afglobal.exe` first
2. Launch a DirectX 11/12 or Vulkan game
3. Check tray for injection notification
4. Verify presence of `afglobal_hook.dll` in game process:
   ```
   # Use Task Manager:
   # Right-click game process → Properties → Detailed/Image Tabs
   # Or use: wmic process where name="game.exe" get modules
   ```

## Building the Installer

### Prerequisites

- Inno Setup 6.0 or later
- Successfully built afglobal.exe and afglobal_hook.dll

### Build Installer

```bash
# Option 1: Using batch script
build_installer.bat

# Option 2: Using Inno Setup directly
iscc installer\afglobal_installer.iss

# Output: installer\output\AFGlobalFGSetup.exe
```

### Test Installer

```bash
# Run installer
installer\output\AFGlobalFGSetup.exe

# Verify installation:
# - Files copied to C:\Program Files\AMDGlobalFG\
# - Tray icon appears
# - Registry entry created for startup
```

## Advanced Build Options

### Build with Debug Symbols

```bash
cmake --build . --config Debug
```

### Enable Verbose Output

```bash
cmake --build . --verbose
```

### Build Specific Target Only

```bash
cmake --build . --target afglobal_exe --config Release
cmake --build . --target afglobal_hook --config Release
```

## Cleaning Build Files

```bash
# Remove all build artifacts
rmdir /s /q build

# Recreate from scratch
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Setting Up for Development

### Visual Studio Setup

1. **Open Project in VS**
   ```
   File → Open Folder → Select project root
   ```

2. **CMake Project Window**
   ```
   View → CMake Project Explorer
   ```

3. **Configure Debug**
   ```
   Debug → Debugger → Windows Debugger
   Debug → Debug and Launch Settings → Select afglobal_exe
   ```

4. **Set Breakpoints and Run**
   ```
   F5 to debug main executable
   ```

### IntelliSense & Code Analysis

- Visual Studio automatically indexes CMake projects
- Intellisense works for C++ files
- Navigation: F12 (Go to Definition)
- Find References: Shift+F12

### Code Formatting

Visual Studio has built-in C++ formatting:
- Right-click → Format Document
- Tools → Options → Text Editor → C/C++ → Formatting

## Continuous Integration

### GitHub Actions Example

Create `.github/workflows/build.yml`:

```yaml
name: Build AFGlobalFG

on: [push, pull_request]

jobs:
  build:
    runs-on: windows-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install CMake
      run: choco install cmake
      
    - name: Install MSVC
      uses: ilammy/msvc-toolset@v1
      with:
        toolset: 14.3
    
    - name: Build
      run: |
        mkdir build
        cd build
        cmake -G "Visual Studio 17 2022" -A x64 ..
        cmake --build . --config Release
    
    - name: Upload Artifacts
      uses: actions/upload-artifact@v3
      with:
        name: afglobal-build
        path: build/bin/
```

## Performance Benchmarking

### Measure Build Time

```batch
@echo off
setlocal enabledelayedexpansion

REM Record start time
for /f "tokens=2-4 delims=/ " %%a in ('date /t') do (set mydate=%%c-%%a-%%b)
for /f "tokens=1-2 delims=/:" %%a in ('time /t') do (set mytime=%%a%%b)
set start_time=!mydate! !mytime!

REM Build
cd build
cmake --build . --config Release

REM Record end time
for /f "tokens=2-4 delims=/ " %%a in ('date /t') do (set mydate=%%c-%%a-%%b)
for /f "tokens=1-2 delims=/:" %%a in ('time /t') do (set mytime=%%a%%b)
set end_time=!mydate! !mytime!

echo Build started: !start_time!
echo Build ended: !end_time!
```

## Submitting Bug Reports

If build fails:

1. **Collect Information**:
   - Windows version: `winver`
   - Visual Studio version
   - CMake version: `cmake --version`
   - MinHook version
   - Full error output (copy entire build log)

2. **Reproduce with Clean Build**:
   ```bash
   rmdir /s /q build
   mkdir build
   cd build
   cmake -G "Visual Studio 17 2022" -A x64 ..
   cmake --build . --config Release 2>&1 | tee build.log
   ```

3. **Submit Issue with**:
   - `build.log` file
   - System info
   - Steps to reproduce
   - What you expected vs what happened

## Next Steps

After successful build:

1. **Deploy to x64 Windows Machine**
   - Copy `afglobal.exe` and `afglobal_hook.dll` to same directory
   - Run `afglobal.exe`

2. **Create Installer**
   ```bash
   build_installer.bat
   ```

3. **Test Injection**
   - Launch a DirectX game
   - Verify injection notification
   - Check FPS improvement

4. **Read QUICKSTART.md for Usage**
   - Configuration options
   - System tray menu
   - Hotkey usage

## Getting Help

- **Documentation**: See [README.md](README.md) and [ARCHITECTURE.md](ARCHITECTURE.md)
- **Quick Start**: See [QUICKSTART.md](QUICKSTART.md)
- **Issues**: Report to GitHub issues with build.log
- **Build Script Errors**: Run with verbose flags:
  ```bash
  cmake --debug-output -DCMAKE_VERBOSE_MAKEFILE=ON ..
  ```
