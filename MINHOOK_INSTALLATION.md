# MinHook Installation Guide

## Current Build Status

✅ **afglobal.exe** - Built successfully
❌ **afglobal_hook.dll** - Requires MinHook library

The main executable builds without MinHook, but the injectable DLL requires MinHook for DXGI hooking functionality.

## Option 1: Install MinHook via vcpkg (Recommended)

### Step 1: Install vcpkg

```powershell
# Clone vcpkg repository
cd C:\
git clone https://github.com/Microsoft/vcpkg.git

# Navigate to vcpkg directory
cd vcpkg

# Bootstrap vcpkg
.\bootstrap-vcpkg.bat

# (Optional) Add vcpkg to PATH for system-wide access
# Edit system environment variables and add C:\vcpkg to PATH
```

### Step 2: Install MinHook

```powershell
# Install MinHook for x64 Windows
C:\vcpkg\vcpkg install minhook:x64-windows
```

### Step 3: Rebuild with MinHook

```powershell
cd "C:\Users\kchan\OneDrive\Desktop\Buss\DLSS AMD"

# Clear existing build
Remove-Item build -Recurse -Force

# Build with MinHook enabled
.\build.ps1 -UseVcpkg -VcpkgRoot "C:\vcpkg"
```

The build script will:
1. Detect vcpkg installation
2. Configure CMake with vcpkg toolchain
3. Enable MinHook support
4. Build both afglobal.exe and afglobal_hook.dll

## Option 2: Manual vcpkg Installation

If vcpkg bootstrap fails, try manual installation:

```powershell
cd C:\vcpkg

# Run CMake manually
cmake . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release

# Set as package provider
.\vcpkg integrate install
```

## Option 3: Build with vcpkg Manifest (Future Enhancement)

Create a `vcpkg.json` file in the project root to auto-fetch MinHook:

```json
{
  "dependencies": [
    "minhook",
    "nlohmann-json"
  ]
}
```

Then build with:
```powershell
cmake . -B build -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake"
cmake --build build --config Release
```

## Troubleshooting

### "vcpkg not found"
- Ensure vcpkg is installed: `C:\vcpkg\vcpkg.exe --version`
- Add to PATH: `[Environment]::SetEnvironmentVariable('Path', $env:Path + ';C:\vcpkg', 'User')`
- Restart PowerShell after adding to PATH

### MinHook installation fails
```powershell
# Update vcpkg
cd C:\vcpkg
git pull
.\bootstrap-vcpkg.bat

# Retry installation
.\vcpkg install minhook:x64-windows
```

### CMake can't find MinHook
- Verify installation: `vcpkg list | Select-String minhook`
- Try explicit path in build command:
```powershell
cmake -DENABLE_MINHOOK=ON -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake" ..
```

### "Multiple toolsets requested"
- Clear build directory: `Remove-Item build\CMake* -Force -Recurse`
- Avoid mixing generators: Always clear build before switching between vcpkg/non-vcpkg builds

## Current Build Output

| File | Status | Location |
|------|--------|----------|
| `afglobal.exe` | ✅ Built | `build\bin\afglobal.exe` |
| `afglobal_hook.dll` | ⏳ Needs MinHook | Requires vcpkg steps above |
| `afglobal_config.json` | ✅ Copied | `build\bin\afglobal_config.json` |

## Next Steps After Installing MinHook

1. **Install MinHook** via vcpkg (Step 1-2 above)
2. **Rebuild project**:
   ```powershell
   cd "C:\Users\kchan\OneDrive\Desktop\Buss\DLSS AMD"
   .\build.ps1 -UseVcpkg -VcpkgRoot "C:\vcpkg"
   ```
3. **Verify output**:
   ```powershell
   Get-Item build\bin\*.dll, build\bin\*.exe
   ```
4. **Test the application**:
   ```powershell
   & build\bin\afglobal.exe
   ```

## For Immediate Testing

You can test afglobal.exe without the DLL:

```powershell
# The exe will work in monitor-only mode
& build\bin\afglobal.exe

# Note: Frame generation requires the DLL (afglobal_hook.dll)
# So injection won't occur until MinHook is installed
```

## Architecture Note

- **afglobal.exe**: Process monitor and system tray (works without MinHook)
- **afglobal_hook.dll**: DXGI hooking and frame interpolation (requires MinHook)

The DLL injection system requires MinHook for intercepting DirectX calls. Without it, the system can't inject the DLL into running games.

---

For more information, see:
- [MinHook GitHub](https://github.com/TsudaKageyu/minhook)
- [vcpkg GitHub](https://github.com/Microsoft/vcpkg)
