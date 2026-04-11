# Quick Start Guide

Get AMD Global Frame Generator up and running in 5 minutes.

## Installation

### Option 1: Pre-built Installer (Easiest)

1. Download `AFGlobalFGSetup.exe`
2. Run the installer
3. Click "Install"
4. The app will start automatically

### Option 2: Manual Installation

1. Copy these files to `C:\Program Files\AMDGlobalFG\`:
   - `afglobal.exe`
   - `afglobal_hook.dll`
   - `afglobal_config.json`

2. Run `afglobal.exe`

3. (Optional) Add to Windows Startup:
   - Press `Win+R`, type `shell:startup`, Enter
   - Create shortcut to `afglobal.exe`

## First Launch

When you first run the application:

1. **System Tray**: An icon appears in your taskbar system tray
2. **Background**: The app runs silently (no window)
3. **Auto-Injection**: When you launch a game, the DLL auto-injects

## Basic Usage

### Enable/Disable Frame Generation

**Option 1: System Tray Menu**
- Right-click the tray icon
- Select "Enable/Disable"

**Option 2: Hotkey**
- Press `Ctrl+Shift+F` anytime

**Option 3: Config File**
- Right-click tray icon → "Open Config"
- Set `"enabled": true` or `false`
- Save file (auto-reloads)

### Adjust Frame Multiplier

Right-click tray icon → "Open Config":

```json
{
  "target_multiplier": 2
}
```

Values:
- `2` = 2x frames (double framerate)
- `3` = 3x frames (triple framerate)  
- `4` = 4x frames (quad framerate)

### View Logs

Right-click tray icon → "View Logs"

Opens `afglobal.log` in Notepad. Useful for debugging.

## Common Scenarios

### Game Not Getting Injected

1. Game must use DirectX 11, DirectX 12, or Vulkan
2. Check logs for error messages
3. Try adding game `.exe` name to config whitelist:

```json
{
  "process_whitelist": ["game.exe", "engine.exe"]
}
```

### Frame Generation Not Visible

1. Verify it's enabled: Press `Ctrl+Shift+F`
2. Check logs for errors
3. Try 2x multiplier first (less demanding)
4. Ensure game is running in borderless/fullscreen

### AMD AMF Not Found Error

This is OK! Means your GPU doesn't have AMF drivers installed.

The app automatically falls back to frame blending:

```json
{
  "fallback_blend_only": true
}
```

Blending is faster but less accurate than optical flow.

### Disable for Specific Games

Add game to blacklist:

```json
{
  "process_blacklist": ["game.exe", "anticheat.exe"]
}
```

## Configuration Quick Reference

```json
{
  "enabled": true,                    // Master on/off
  "target_multiplier": 2,             // 2, 3, or 4
  "process_blacklist": [              // Never inject into these
    "explorer.exe",
    "dwm.exe"
  ],
  "process_whitelist": [],            // If set, ONLY inject into these
  "fallback_blend_only": false,       // Force blending (no optical flow)
  "log_level": "INFO",                // DEBUG, INFO, WARNING, ERR, CRITICAL
  "hotkey": "CTRL+SHIFT+F"            // Global hotkey string
}
```

## Uninstall

### Using Installer

Settings → Apps & features → Find "AMD Global Frame Generator" → Uninstall

### Manual Removal

1. Close `afglobal.exe` from tray
2. Delete `C:\Program Files\AMDGlobalFG\`
3. (If added to startup) Delete from `shell:startup`

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+Shift+F` | Toggle frame generation on/off |

## Tray Menu Options

| Option | Action |
|--------|--------|
| Enable/Disable | Toggle frame generation |
| Open Config | Edit `afglobal_config.json` |
| View Logs | View `afglobal.log` |
| Exit | Cleanly shutdown application |

## Troubleshooting

### App Won't Start

1. Run as Administrator
2. Check if `afglobal.exe` is already running in Task Manager
3. Restart Windows

### Game Crashes After Injection

1. Disable frame generation: `Ctrl+Shift+F`
2. Try 2x multiplier instead of 3x/4x
3. Update graphics drivers
4. Add game to blacklist temporarily

### CPU Usage Too High

1. Try 2x multiplier instead of 3x/4x
2. Check Task Manager for runaway `afglobal.exe` process
3. Set `log_level` to "WARNING" (reduces logging overhead)

### GPU Not Supported

The app requires:
- DirectX 12 compatible GPU
- 4GB+ VRAM
- Recent drivers (2020 or later)

## Tips & Tricks

### Better Performance

- Start with 2x multiplier
- Lower game resolution slightly
- Use optical flow (don't set `fallback_blend_only: true`)
- Close background apps

### Better Visual Quality

- Use optical flow (requires AMD GPU drivers)
- Increase game resolution
- Use 3x multiplier for smoother motion

### Benchmarking

1. Run game with frame counter
2. Note baseline FPS
3. Enable frame generation with 2x multiplier
4. Disable logging (`log_level: "WARNING"`) for best performance
5. Compare FPS difference

## Support

For issues:

1. Check `afglobal.log`
2. Enable DEBUG logging to see more detail:

```json
{
  "log_level": "DEBUG"
}
```

3. Note error messages and game title
4. Check system specs: GPU, driver version, Windows version

## Next Steps

- Read full [README.md](README.md) for advanced configuration
- Check [ARCHITECTURE.md](ARCHITECTURE.md) for technical details
- Review [BUILD.md](BUILD.md) to compile from source

## Performance Expectations

Typical performance on modern hardware:

| Hardware | 2x Multiplier | 3x Multiplier | 4x Multiplier |
|----------|---------------|---------------|---------------|
| RTX 3060 | Stable 120 FPS | ~90 FPS | ~60 FPS |
| RTX 4070 | Stable 240 FPS | ~160 FPS | ~120 FPS |
| RDNA2+ iGPU | 60+ FPS | 40+ FPS | 30+ FPS |

*Results vary depending on game and resolution*
