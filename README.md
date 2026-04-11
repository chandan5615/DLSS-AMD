# AMD Global Frame Generator

A production-ready C++ Windows application that works as a global multi-frame generation injector for AMD iGPUs. This application monitors running processes, injects a hooking DLL into DirectX/Vulkan games, and uses AMD AMF's optical flow capabilities to generate extra frames between real frames, effectively multiplying framerate.

## Features

- **System Tray Application**: Runs silently in the background with system tray integration
- **Process Monitoring**: Continuously monitors all running processes for new game launches
- **Automatic DLL Injection**: Injects the hooking DLL into detected graphics applications
- **Frame Generation**: Generates 2x, 3x, or 4x frames using optical flow or blending
- **Global Hotkey**: Ctrl+Shift+F to toggle frame generation at runtime
- **Named Pipe Communication**: Real-time communication between executable and DLL
- **Rotating Logs**: Comprehensive logging for debugging and monitoring
- **Configuration Management**: JSON-based configuration with hot-reload support

## Project Structure

```
AFGlobalFrameGenerator/
├── src/
│   ├── common/              # Shared code
│   │   ├── config.h/cpp
│   │   ├── constants.h
│   │   └── logger.h/cpp
│   ├── afglobal_exe/        # Main executable
│   │   ├── main.cpp
│   │   ├── tray_app.h/cpp
│   │   ├── process_monitor.h/cpp
│   │   ├── injector.h/cpp
│   │   └── named_pipe_client.h/cpp
│   └── afglobal_dll/        # Injectable DLL
│       ├── dllmain.cpp
│       ├── dxgi_hooks.h/cpp
│       ├── frame_interpolator.h/cpp
│       ├── amf_optical_flow.h/cpp
│       └── named_pipe_server.h/cpp
├── installer/
│   └── afglobal_installer.iss
├── CMakeLists.txt
├── afglobal_config.json
└── README.md
```

## Requirements

- **OS**: Windows 10 or Windows 11 (x64 only)
- **Build Tools**: 
  - Visual Studio 2019 or later with C++ workload
  - CMake 3.20 or later
  - Inno Setup 6.0+ (for building installer)
- **Dependencies**:
  - MinHook 3.3.2 (auto-fetched via CMake)
  - nlohmann/json (auto-fetched via CMake)
  - DirectX 12 SDK headers
  - AMD AMF SDK (optional, for optical flow)

## Building from Source

### Prerequisites

1. **Visual Studio 2019 or 2022** with C++ workload installed
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Select "Desktop development with C++" workload

2. **CMake 3.20 or later**
   - Download from: https://cmake.org/download/
   - Add to system PATH for command-line access

3. **DirectX 12 SDK headers** (included with Visual Studio)

4. **Git** (optional, for cloning the repository)
   - Download from: https://git-scm.com/downloads

### Quick Build (Recommended)

**Windows PowerShell (Recommended)**:

```powershell
# Navigate to project directory
cd "C:\path\to\AMD Global Frame Generator"

# Run the automated build script
.\build.ps1
```

The script will:
- Auto-detect installed Visual Studio/CMake
- Configure and build the project
- Copy binaries to `build\bin\` directory

### Manual Build Steps

If you prefer manual control:

```powershell
# Create and enter build directory
mkdir build
cd build

# Configure with CMake (auto-detects Visual Studio version)
cmake -G "Visual Studio 17 2022" -A x64 ..

# Alternative for Visual Studio 2019:
# cmake -G "Visual Studio 16 2019" -A x64 ..

# Build in Release mode
cmake --build . --config Release

# Output files:
# - build/Release/afglobal.exe
# - build/Release/afglobal_hook.dll
```

### Build with vcpkg (Optional MinHook)

If you have vcpkg installed for dependency management:

```powershell
cmake -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake" `
  ..
cmake --build . --config Release
```

### Troubleshooting Build Issues

**Issue: CMake not found**
- Solution: Add CMake to PATH or specify full path: `C:\Program Files\CMake\bin\cmake.exe`

**Issue: Visual Studio not detected**
- Solution: Ensure VS 2019 or 2022 is installed with C++ workload
- Try specifying generator explicitly: `-G "Visual Studio 17 2022"`

**Issue: BUILD FAILED errors**
- Check log output for missing dependencies
- Ensure you have write permissions in the build directory
- Try deleting `build` folder and reconfiguring from scratch

## Configuration

Configure application behavior by editing `afglobal_config.json`:

**Default Configuration**:
```json
{
  "enabled": true,
  "target_multiplier": 2,
  "process_blacklist": ["explorer.exe", "dwm.exe"],
  "process_whitelist": [],
  "fallback_blend_only": false,
  "log_level": "INFO",
  "hotkey": "CTRL+SHIFT+F"
}
```

### Configuration Reference

| Field | Type | Values | Description |
|-------|------|--------|-------------|
| `enabled` | boolean | `true` / `false` | Enable/disable frame generation globally |
| `target_multiplier` | integer | 2, 3, 4 | Frame multiplier: 2x = 60→120 fps, 3x = 60→180 fps, 4x = 60→240 fps |
| `process_blacklist` | array | process names | Processes to NEVER inject into (e.g., `["explorer.exe", "dwm.exe"]`) |
| `process_whitelist` | array | process names | If not empty, ONLY inject into these processes |
| `fallback_blend_only` | boolean | `true` / `false` | Use simple blending instead of optical flow (faster but less quality) |
| `log_level` | string | DEBUG, INFO, WARNING, ERR, CRITICAL | Logging verbosity level |
| `hotkey` | string | Key combination | Global hotkey to toggle (e.g., `"CTRL+SHIFT+F"`) |

### Configuration Examples

**High Performance (Lower Latency)**:
```json
{
  "target_multiplier": 2,
  "fallback_blend_only": true,
  "log_level": "WARNING"
}
```

**High Quality (Maximum Interpolation)**:
```json
{
  "target_multiplier": 4,
  "fallback_blend_only": false,
  "log_level": "INFO"
}
```

**Whitelist Specific Games**:
```json
{
  "process_whitelist": ["cyberpunk2077.exe", "starfield.exe"],
  "target_multiplier": 3
}
```

### Changing Configuration at Runtime

1. Right-click tray icon → "Open Config"
2. Edit `afglobal_config.json`
3. Save the file
4. Changes take effect immediately (no restart needed)
5. Check log for confirmation

## Installation

### Method 1: Manual Installation (Recommended for Development)

**Step 1: Build the project**
```powershell
.\build.ps1
```

**Step 2: Setup installation directory**
```powershell
# Create program directory
New-Item -ItemType Directory -Path "C:\Program Files\AMDGlobalFG" -Force
```

**Step 3: Copy files**
```powershell
# Copy binaries and config
Copy-Item "build\bin\afglobal.exe" "C:\Program Files\AMDGlobalFG\"
Copy-Item "build\bin\afglobal_hook.dll" "C:\Program Files\AMDGlobalFG\"
Copy-Item "afglobal_config.json" "C:\Program Files\AMDGlobalFG\"
```

**Step 4: Run the application**
```powershell
& "C:\Program Files\AMDGlobalFG\afglobal.exe"
```

**Step 5 (Optional): Add to Windows Startup**

To run automatically at startup:

1. Press `Win + R`, type `shell:startup`, press Enter
2. Create a shortcut to `afglobal.exe`
3. Or run PowerShell as Administrator:

```powershell
$shortcutPath = "$([Environment]::GetFolderPath('Startup'))\AMD Global FG.lnk"
$targetPath = "C:\Program Files\AMDGlobalFG\afglobal.exe"
$WshShell = New-Object -ComObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut($shortcutPath)
$Shortcut.TargetPath = $targetPath
$Shortcut.Save()
```

### Method 2: Using Installer (Distribution)

1. Build the project: `.\build.ps1`
2. Files are automatically placed in `build\bin\`
3. Install Inno Setup: https://jrsoftware.org/isdl.php
4. Build installer:
   ```powershell
   & "C:\Program Files (x86)\Inno Setup 6\iscc.exe" installer\afglobal_installer.iss
   ```
5. Run generated installer: `installer\AFGlobalFGSetup.exe`

## Quick Start Guide

Get AMD Global Frame Generator up and running in 5 minutes.

### 1. Build (1 minute)

```powershell
cd "C:\Path\To\DLSS AMD"
.\build.ps1
```

Wait for "Build complete" message.

### 2. Deploy (1 minute)

```powershell
# Create installation directory
New-Item -ItemType Directory -Path "C:\Program Files\AMDGlobalFG" -Force

# Copy the files
Copy-Item "build\bin\afglobal.exe" "C:\Program Files\AMDGlobalFG\"
Copy-Item "build\bin\afglobal_hook.dll" "C:\Program Files\AMDGlobalFG\"
Copy-Item "afglobal_config.json" "C:\Program Files\AMDGlobalFG\" -Force
```

### 3. Launch (30 seconds)

```powershell
& "C:\Program Files\AMDGlobalFG\afglobal.exe"
```

You should see a tray icon appear in the bottom-right corner (notification area).

### 4. Configure (2 minutes)

Right-click the tray icon and select **"Open Config"** to customize:

**Essential settings for 60→120 FPS**:
```json
{
  "frame_generation_enabled": true,
  "frame_multiplier": 2,
  "hotkey_toggle": "ctrl+shift+f"
}
```

**For high-performance systems**:
```json
{
  "frame_generation_enabled": true,
  "frame_multiplier": 4,
  "optical_flow_preset": "high_quality"
}
```

**For integration with existing hooks** (if you have other mods):
```json
{
  "fallback_blend_only": true,
  "frame_multiplier": 2
}
```

### 5. Test (30 seconds)

1. Launch your favorite DirectX 11/12 or Vulkan game
2. Application will automatically inject the DLL
3. Press **Ctrl+Shift+F** to toggle frame generation
4. Check FPS counter in game (should be ~2x your base framerate)

### Troubleshooting Quick Fixes

| Issue | Quick Fix |
|-------|-----------|
| Tray icon not visible | Click "Show hidden icons" in tray (bottom-right) |
| DLL not injecting | Update graphics drivers, restart ngame |
| Low performance | Set `frame_multiplier: 2`, enable `fallback_blend_only` |
| Game-specific issues | Add game.exe to `whitelisted_processes` in config |
| Need more help | See Troubleshooting section below |

### Next Steps

- **For per-game configurations**: See "Per-Game Configuration" in Usage section
- **For advanced settings**: See Configuration reference below
- **For common issues**: See Troubleshooting section

---

## Usage

### Running the Application

**From Command Line**:
```powershell
C:\Program Files\AMDGlobalFG\afglobal.exe
```

**From Windows Explorer**:
1. Navigate to `C:\Program Files\AMDGlobalFG\`
2. Double-click `afglobal.exe`

**From PowerShell**:
```powershell
& "C:\Program Files\AMDGlobalFG\afglobal.exe"
```

**What Happens on Launch**:
1. Application starts silently (no console window)
2. System tray icon appears in the notification area (bottom-right corner)
3. Application begins monitoring all running processes
4. When a DirectX/Vulkan game launches, DLL auto-injects automatically
5. Frame generation begins based on configuration settings

### Initial Setup

**First Time Running**:
1. Look for the system tray icon (may be in the "hidden icons" area)
2. Right-click the tray icon to open the menu
3. Click "Open Config" to customize settings
4. Modify `afglobal_config.json` as needed
5. Configuration changes take effect immediately

**Default Configuration**:
- Frame generation: 2x multiplier
- Mode: Optical flow (if available) with blending fallback
- Hotkey: Ctrl+Shift+F to toggle
- Log level: INFO
- Blacklist: explorer.exe, dwm.exe (Windows system processes)

### System Tray Controls

**Right-click the tray icon** to open the context menu:

| Option | Function |
|--------|----------|
| **Enable/Disable** | Toggle frame generation on/off globally |
| **Open Config** | Opens `afglobal_config.json` in default text editor |
| **View Logs** | Opens `afglobal.log` in default text editor |
| **Exit** | Cleanly shutdown the application |

**Left-click the tray icon** to show/hide main window (if enabled in config)

### Global Hotkey Control

**Ctrl+Shift+F** - Toggle frame generation on/off
- Works while any window has focus
- Useful for quick enable/disable during gameplay
- Can be customized in `afglobal_config.json`

### Per-Game Control

To enable frame generation only for specific games:

1. Open `afglobal_config.json`
2. Add game executable names to `process_whitelist`:

```json
{
  "process_whitelist": [
    "game1.exe",
    "game2.exe"
  ]
}
```

3. Save and restart the application
4. DLL will only inject into whitelisted processes

## Logging and Debugging

### Log File Location

- **Path**: `afglobal.log` in the installation directory
  - Default: `C:\Program Files\AMDGlobalFG\afglobal.log`

### Log Levels

Configure verbosity in `afglobal_config.json`:

| Level | Use Case |
|-------|----------|
| **CRITICAL** | Only system-critical failures |
| **ERR** | Error conditions (DLL injection failures, missing files) |
| **WARNING** | Potential issues (unsupported games, missing drivers) |
| **INFO** | General operation info (game detected, DLL injected) - *Default* |
| **DEBUG** | Detailed diagnostic information (all function calls, timing) |

### Viewing Logs

**Via Tray Menu**:
1. Right-click system tray icon
2. Click "View Logs"

**Manually**:
```powershell
# Open with default text editor
Invoke-Item "C:\Program Files\AMDGlobalFG\afglobal.log"

# Follow log in real-time (PowerShell)
Get-Content "C:\Program Files\AMDGlobalFG\afglobal.log" -Tail 10 -Wait
```

### Common Log Patterns

**Successful Operation**:
```
[INFO] Game process detected: Game.exe
[INFO] DLL injection successful
[INFO] Frame generation started (2x multiplier)
```

**Troubleshooting Clues**:
```
[ERR] Failed to inject DLL: Access Denied
[WRN] AMD AMF not available, using blending fallback
[ERR] Named pipe communication failed
```

## Technical Architecture

### Main Executable (`afglobal.exe`)

- **TrayApp**: System tray window and menu handling
- **ProcessMonitor**: Continuously enumerates running processes
- **Injector**: Implements DLL injection via CreateRemoteThread
- **NamedPipeClient**: Communicates with injected DLL for control
- **ConfigManager**: Manages shared configuration

### Injectable DLL (`afglobal_hook.dll`)

- **DXGIHooks**: Hooks IDXGISwapChain::Present using MinHook
- **FrameInterpolator**: Manages frame buffer and interpolation logic
- **AMFOpticalFlow**: Interface to AMD AMF for optical flow
- **NamedPipeServer**: Receives commands from main executable
- **ConfigManager**: Loads configuration (shared with exe)

### Frame Interpolation

The DLL intercepts Present() calls and:

1. **Capture Current Frame**: Extracts back buffer from swap chain
2. **Estimate Motion**: Uses AMD AMF optical flow to compute motion vectors
3. **Warp & Interpolate**: Warps previous frame toward current using vectors
4. **Generate Frame**: Creates interpolated frame at intermediate timestamps
5. **Present Sequence**: 
   - Present interpolated frame 1
   - Present interpolated frame 2 (if 3x/4x mode)
   - Present interpolated frame 3 (if 4x mode)
   - Present original current frame

### Fallback Blending

If AMF optical flow is unavailable:

- Simple linear interpolation between previous and current frame
- Fast but less accurate than optical flow
- Enabled by setting `fallback_blend_only: true`

## Communication Protocol

### Named Pipe: `\\.\pipe\afglobal_command`

**Message Format:**
```
[4 bytes: Command ID][4 bytes: Parameter]
```

**Commands:**

- `1`: TOGGLE_FRAME_GEN - Toggle frame generation on/off
- `2`: GET_STATUS - Query current status
- `3`: SET_MULTIPLIER - Set frame multiplier (param: 2-4)
- `4`: SHUTDOWN - Gracefully shutdown

**Responses:**

- `0`: SUCCESS
- `1`: FAILURE
- `2`: UNKNOWN_CMD
- `3`: INVALID_PARAM

## Troubleshooting

This section helps diagnose and resolve common issues. Always check the log file first (`afglobal.log`) for detailed error information.

### Build Issues

#### CMake Not Found

**Error**: `'cmake' is not recognized as an internal or external command`

**Solutions**:
1. Install CMake from [cmake.org](https://cmake.org/download/)
2. Add CMake to PATH during installation
3. Or use the `build.ps1` script which auto-detects installation
4. Restart terminal/PowerShell after installing

#### Visual Studio Not Found

**Error**: `Visual Studio 2022+ not found` or CMAKE_GENERATOR error

**Solutions**:
1. Install Visual Studio 2022 Community (free) from [visualstudio.com](https://visualstudio.com)
2. During installation, select "Desktop development with C++"
3. Ensure workload includes "C++ build tools"
4. Run `build.ps1` which auto-detects VS installation

#### Permission Denied

**Error**: `Access denied` when writing to build directory

**Solutions**:
1. Run PowerShell as Administrator
2. Or run `build.ps1` with `-Admin` flag
3. Ensure build directory is not in OneDrive/CloudSync folder
4. Use local disk (`C:\`) instead of network drive

### Runtime Issues

#### DLL Not Injecting

**Symptoms**: Game runs but frame generation doesn't activate; logs show "DLL injection failed"

**Diagnosis**:
1. Check `afglobal.log` for specific error message
2. Verify `afglobal_hook.dll` exists in same directory as `afglobal.exe`:
   ```powershell
   Get-Item "C:\Program Files\AMDGlobalFG\afglobal_hook.dll"
   ```
3. Confirm game process is 64-bit (x64):
   ```powershell
   Get-Process | Where-Object {$_.ProcessName -like "*game*"} | Select Name, Handles
   ```

**Solutions**:
1. **Update graphics drivers**:
   - AMD: Download from [radeondriver.com](https://radeondriver.com)
   - NVIDIA: Download from [nvidia.com/Download](https://nvidia.com/Download)
   - Intel: Download from intel.arc.com
   
2. **Add game to whitelist** in config.json:
   ```json
   {
     "whitelisted_processes": [
       "game.exe",
       "game_launcher.exe"
     ]
   }
   ```

3. **Verify DirectX/Vulkan compatibility**:
   - Game must use DirectX 11, 12, or Vulkan
   - Use [GPU-Z](https://www.techpowerup.com/download/techpowerup-gpu-z/) to verify rendered API
   - Or check game settings→graphics→rendering API

4. **Check for anti-cheat conflicts**:
   - Some games block DLL injection
   - Disable anti-cheat if game offers offline mode
   - Try in single-player before multiplayer

#### Frame Generation Not Working

**Symptoms**: Application runs but frames aren't interpolated; Ctrl+Shift+F has no effect

**Diagnosis**:
1. Check if enabled in config:
   ```json
   {
     "frame_generation_enabled": true,
     "hotkey_toggle": "ctrl+shift+f"
   }
   ```

2. Check logs for capability detection:
   ```powershell
   Select-String "optical_flow_available\|frame_gen_available" afglobal.log
   ```

**Solutions**:
1. **Press Ctrl+Shift+F** to toggle frame generation
2. **Check log verbosity** - set to INFO or DEBUG:
   ```json
   {
     "log_level": "DEBUG"
   }
   ```
3. **Verify AMD capabilities**:
   - Open `afglobal.log`
   - Search for "optical_flow_available" or "AMD AMF initialized"
   - If false, install AMD GPU drivers or enable fallback
4. **Enable fallback blend mode**:
   ```json
   {
     "fallback_blend_only": true
   }
   ```
5. **Reduce frame multiplier** if performance is an issue:
   ```json
   {
     "frame_multiplier": 2
   }
   ```

#### Application Crashes

**Symptoms**: `afglobal.exe` crashes on launch or during gameplay

**Diagnosis**:
1. **Check Windows Event Viewer** for system errors:
   ```powershell
   Get-EventLog -LogName System -Newest 10 -Source ".NET Runtime" | Format-List
   ```

2. **Check afglobal.log** for stack trace or error message

3. **Test in Safe Mode** (if driver-related):
   - Restart in Safe Mode
   - Attempt to run afglobal.exe

**Solutions**:
1. **Update graphics drivers** - outdated drivers are a common cause
2. **Update Windows** - Run Windows Update
3. **Disable experimental features** in config:
   ```json
   {
     "frame_generation_enabled": false,
     "fallback_blend_only": true,
     "frame_multiplier": 2
   }
   ```
4. **Check system resources**:
   - Ensure 500MB+ free disk space
   - Ensure 1GB+ free RAM
   - Check GPU VRAM (should have 2GB+ free)
5. **Reinstall application**:
   - Uninstall completely (see Uninstallation section)
   - Delete config.json
   - Reinstall fresh

#### High CPU Usage

**Symptoms**: CPU usage >20% while idle or during gameplay

**Diagnosis**:
1. Check what's consuming CPU in Task Manager:
   ```powershell
   Get-Process | Where-Object {$_.ProcessName -eq "afglobal"} | Select-Object Name, CPU, Handles, WorkingSet
   ```

2. Check logs for continuous operations:
   ```powershell
   Select-String "ERROR\|WARNING" afglobal.log | Tail -20
   ```

**Solutions**:
1. **Reduce polling frequency** in config:
   ```json
   {
     "process_poll_interval_ms": 2000
   }
   ```
2. **Disable unnecessary features**:
   ```json
   {
     "frame_generation_enabled": false,
     "fallback_blend_only": true
   }
   ```
3. **Update drivers** - inefficient driver code can cause high CPU

#### AMD AMF Not Available

**Error**: `AMD AMF initialization failed` or optical flow fails to load

**Solutions**:
1. **Install AMD GPU drivers**:
   - Download from [radeondriver.com](https://radeondriver.com)
   - Or use AMD Driver Auto-Detect tool
   - Restart computer after installation

2. **Use fallback blend mode** if AMF unavailable:
   ```json
   {
     "fallback_blend_only": true
   }
   ```

3. **Verify AMD GPU detected**:
   ```powershell
   Get-WmiObject Win32_VideoController
   ```

4. **Check GPU capabilities**:
   - Use [GPU-Z](https://www.techpowerup.com/download/techpowerup-gpu-z/)
   - Verify GPU supports video encoding (HEVC/H.264)

#### Game-Specific Issues

**Unreal Engine games**:
- May require DXGI hooking already in place
- Try: `set DefaultEngine.Rendering.bUseD3D12 False` in config

**Source engine games** (Valve):
- Ensure DirectX 11 or 12 mode enabled
- May need to whitelist game executable

**Proprietary engines**:
- Check compatibility list on project site
- Some engines have anti-hooking protections

### Diagnostic Tools

#### Enable Debug Logging

```json
{
  "log_level": "DEBUG",
  "log_file": "afglobal_debug.log"
}
```

Then analyze logs:
```powershell
# View last 50 errors
Select-String "ERROR" afglobal_debug.log | Tail -50

# View timestamps and messages
Select-String "^\d{4}-\d{2}-\d{2}" afglobal_debug.log | Tail -20
```

#### System Information Collection

Run this to collect diagnostics for troubleshooting:
```powershell
# GPU Info
Write-Host "=== GPU Information ===" 
Get-WmiObject Win32_VideoController

# System Info
Write-Host "=== Windows Version ===" 
Get-ComputerInfo -Property WindowsVersion

# Driver Version
Write-Host "=== Display Driver Version ===" 
Get-WmiObject Win32_PnPSignedDriver | Where-Object {$_.DeviceName -like "*Display*"}

# Running Games (example)
Write-Host "=== Running Game Processes ===" 
Get-Process | Where-Object {$_.ProcessName -notlike "system*" -and $_.Handles -gt 100}
```

#### Check Log Severity Distribution

```powershell
# Count errors by type
(Select-String "^\[.*?\]" afglobal.log).Line | Group-Object | Select-Object Count, Name | Sort-Object Count -Descending
```

### Performance Tuning

If experiencing stutters or high frame drops:

```json
{
  "frame_multiplier": 2,
  "optical_flow_preset": "balanced",
  "fallback_blend_only": true,
  "frame_generation_enabled": true
}
```

Adjust in order (test each change):
1. Reduce `frame_multiplier` (2 instead of 4)
2. Change `optical_flow_preset` to `fast` or `balanced`
3. Enable `fallback_blend_only` for lower GPU load
4. Disable `frame_generation_enabled` temporarily to baseline

### Getting Help

If issues persist after trying solutions above:

1. **Check project documentation**: See TROUBLESHOOTING_ADVANCED.md
2. **Collect diagnostics**:
   - Run diagnostic script (see above)
   - Save `afglobal.log`
   - Note Windows version, GPU model, driver version, game title
3. **Check GitHub issues**: Similar problems may have solutions
4. **Report issue with**:
   - Complete error message from log
   - Reproduction steps
   - System information (Windows version, GPU, drivers)
   - Latest log file (24 hours before issue)

## Performance Considerations

- **CPU Usage**: ~1-2% idle, ~5-10% while generating frames
- **GPU Usage**: Depends on optical flow complexity
- **Latency**: ~1-2ms per interpolated frame (optical flow) or <1ms (blending)
- **Memory**: ~50-100MB footprint

## Security Notes

- The application requires administrator privileges for process injection
- DLL injection is detected by anti-cheat systems in some games
- Use at your own discretion in competitive online gaming
- Log files may contain privacy-sensitive information

## Uninstallation

### Manual Removal

1. Close `afglobal.exe` from system tray
2. Delete `C:\Program Files\AMDGlobalFG` folder
3. Remove startup registry entry (if added):
   - Run: `regedit`
   - Navigate to: `HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Run`
   - Delete `AMDGlobalFG` entry

### Using Installer

Run: `Control Panel\Programs and Features\AMD Global Frame Generator\Uninstall`

Or use: `Programs and Features` (Add/Remove Programs)

## Building the Installer

```bash
# After building the project:
# 1. Copy binaries to build/bin/
# Copy bin/afglobal.exe build/bin/
# Copy bin/afglobal_hook.dll build/bin/

# 2. Run Inno Setup
iscc installer\afglobal_installer.iss

# Output: AFGlobalFGSetup.exe in installer/
```

## License

This software is provided as-is without warranty. Use at your own discretion.

## Contributing

For bug reports or feature requests, please document:
1. Windows version (10/11)
2. GPU model and drivers
3. Game title and version
4. Error message from `afglobal.log`
5. Steps to reproduce

## Revision History

### v1.0.0 (Release)
- Initial release
- DXGI hooking via MinHook
- Process monitoring and auto-injection
- Named pipe IPC
- System tray UI
- Frame interpolation with optical flow fallback
- Configuration system
- Rotating logs
