# Architecture & Technical Design

Deep dive into the AMD Global Frame Generator architecture.

## System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      Windows System                          │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────────────────┐    ┌────────────────────────┐ │
│  │   afglobal.exe           │────│ afglobal_hook.dll      │ │
│  │  (Main Process)          │    │  (Game Process)        │ │
│  ├──────────────────────────┤    ├────────────────────────┤ │
│  │ • Tray UI                │    │ • DirectX Hooking      │ │
│  │ • Process Monitor        │    │ • Frame Buffer Mgmt    │ │
│  │ • DLL Injection          │    │ • Optical Flow         │ │
│  │ • Hotkey Handler         │    │ • Named Pipe Server    │ │
│  │ • Named Pipe Client      │────│ • Config Loading       │ │
│  │ • Config Manager         │    │ • Logging              │ │
│  └──────────────────────────┘    └────────────────────────┘ │
│           │                               │                  │
│           │                               │                  │
│           ├──────Named Pipe───────────────┤                  │
│           │ (\\.\pipe\afglobal_command)   │                  │
│           │                               │                  │
│           ├──────Inject (CreateRemoteThread)                 │
│           │                               │                  │
│           └───────Read Config File────────┘                  │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

## Component Architecture

### Main Executable (afglobal.exe)

#### TrayApp (tray_app.h/cpp)
- **Purpose**: System tray UI and application lifecycle
- **Responsibilities**:
  - Create hidden main window (for tray integration)
  - Manage system tray icon and menu
  - Handle hotkey registration (Ctrl+Shift+F)
  - Display balloon notifications
  - Coordinate all other components

- **Key Methods**:
  ```cpp
  Initialize(HINSTANCE)  // Setup tray, hooks, monitors
  Run()                  // Windows message loop
  Shutdown()             // Cleanup and exit
  ToggleFrameGeneration() // Call DLL via named pipe
  ```

- **Message Handling**:
  - `WM_TRAY_ICON`: Right-click menu
  - `WM_HOTKEY`: Ctrl+Shift+F pressed
  - `WM_COMMAND`: Menu item selected
  - `WM_DESTROY`: Application exit

#### ProcessMonitor (process_monitor.h/cpp)
- **Purpose**: Real-time process enumeration and tracking
- **Responsibilities**:
  - Continuously enumerate running processes
  - Detect process creation/termination
  - Filter by graphics API (DirectX/Vulkan)
  - Track process whitelist/blacklist
  - Trigger injection on new game process

- **Algorithm**:
  ```
  Every PROCESS_MONITOR_INTERVAL_MS milliseconds:
    1. EnumProcesses() - get all PIDs
    2. For each PID:
       a. Get process name and path
       b. Check graphics modules (d3d11.dll, d3d12.dll, vulkan.dll)
       c. If new and is game process:
         - Add to tracked_pids_
         - Callback to injector
    3. Clean up dead processes from tracking set
  ```

- **Game Detection Heuristics**:
  - Must have: d3d11, d3d12, or vulkan module loaded
  - Must NOT be in blacklist
  - If whitelist exists, must be in whitelist
  - Typical game processes: `game.exe`, `engine.exe`, etc.

#### Injector (injector.h/cpp)
- **Purpose**: DLL injection into target processes
- **Method**: CreateRemoteThread + LoadLibraryA
- **Responsibilities**:
  - Load target process (PROCESS_ALL_ACCESS)
  - Allocate memory in target (VirtualAllocEx)
  - Write DLL path to target memory
  - Get kernel32.LoadLibraryA address
  - Create remote thread with LoadLibrary
  - Verify injection by checking loaded modules
  - Track injected PIDs for cleanup

- **Injection Flow**:
  ```
  1. OpenProcess(targetPid, PROCESS_ALL_ACCESS)
  2. VirtualAllocEx() - allocate space for DLL path
  3. WriteProcessMemory() - copy DLL path string
  4. GetProcAddress(kernel32, "LoadLibraryA")
  5. CreateRemoteThread(hProcess, LoadLibraryA, remotePath)
  6. WaitForSingleObject() - wait for thread completion
  7. VerifyInjection() - check if DLL loaded successfully
  8. Return success/failure
  ```

- **Verification**:
  - EnumProcessModules to find injected DLL
  - Match by module name (case-insensitive)
  - Confirm module handle is valid

#### NamedPipeClient (named_pipe_client.h/cpp)
- **Purpose**: IPC with injected DLL
- **Protocol**: Named pipe at `\\.\pipe\afglobal_command`
- **Responsibilities**:
  - Connect to DLL's named pipe server
  - Send commands (toggle, status, multiplier)
  - Receive responses
  - Handle retry logic

- **Command Protocol**:
  ```
  Message Format:
  [4 bytes: Command][4 bytes: Parameter]
  
  Commands:
  - TOGGLE_FRAME_GEN: Toggle enable/disable
  - GET_STATUS: Query current state
  - SET_MULTIPLIER: Set frame multiplier (2-4)
  - SHUTDOWN: Clean shutdown
  ```

### Injectable DLL (afglobal_hook.dll)

#### DLLMain (dllmain.cpp)
- **Entry Point**: DLL_PROCESS_ATTACH / DLL_PROCESS_DETACH
- **On Attach**:
  1. Initialize logger
  2. Load configuration
  3. Initialize AMF optical flow
  4. Initialize DXGI hooks with MinHook
  5. Start named pipe server
  6. Initialize frame interpolator

- **On Detach**:
  1. Stop named pipe server
  2. Unhook all APIs
  3. Cleanup frame interpolator
  4. Shutdown AMF

#### DXGIHooks (dxgi_hooks.h/cpp)
- **Purpose**: Hook DXGI swap chain for frame capture
- **Framework**: MinHook library
- **Hooked Functions**:
  - `CreateDXGIFactory`: For GPU capability spoofing
  - `IDXGISwapChain::Present`: Intercept frame presentation
  - `IDXGISwapChain1::Present1`: DirectX 12 variant

- **Present Hook Logic**:
  ```cpp
  HookedPresent(pSwapChain, SyncInterval, Flags) {
    if (config.enabled) {
      // Extract back buffer
      pSwapChain->GetBuffer(0, &backBuffer)
      
      // Process frame for interpolation
      FrameInterpolator.ProcessFrame(backBuffer, config.multiplier)
      
      backBuffer->Release()
    }
    
    // Call original Present
    return OriginalPresent(pSwapChain, SyncInterval, Flags)
  }
  ```

- **Hook Technique**:
  - Create hook with MH_CreateHook
  - Enable with MH_EnableHook
  - Store original function pointer
  - Call original in hooked version

#### FrameInterpolator (frame_interpolator.h/cpp)
- **Purpose**: Frame generation and temporal interpolation
- **Algorithm**:
  ```
  1. On each Present() call:
     a. Capture current backbuffer frame
     b. Store in ring buffer (frame[0], frame[1])
     
  2. If we have previous frame:
     a. Compute optical flow (prev → curr)
     b. For each interpolated frame (1 to multiplier-1):
        - Calculate time parameter t = i/multiplier
        - Warp previous frame by motion*t
        - Generate interpolated frame
        - Present interpolated frame
        
     c. Present current (real) frame
  ```

- **Ring Buffer**:
  - Stores last 2 frames: `frame_buffer_[0]` and `frame_buffer_[1]`
  - Alternates which is used as previous/current
  - Minimizes memory usage

- **Frame blending (Fallback)**:
  ```
  interpolated = prev * (1-w) + curr * w
  where w = current_interp_index / multiplier
  ```

#### AMFOpticalFlow (amf_optical_flow.h/cpp)
- **Purpose**: Motion vector computation via AMD AMF
- **Library**: amfrt64.dll (optional)
- **Responsibilities**:
  - Load AMF runtime at DLL load time
  - Initialize AMF components
  - Compute optical flow between frames
  - Handle fallback (blending) if unavailable

- **AMF Integration**:
  ```cpp
  1. LoadLibrary("amfrt64.dll")
  2. GetProcAddress("AMFInit64")
  3. Create AMD AMF compute context
  4. Load both frames to GPU
  5. Run optical flow algorithm
  6. Download motion vectors
  7. Use vectors to warp frame
  ```

- **Fallback Chain**:
  1. Try optical flow
  2. If unavailable, use frame blending
  3. Blending is always available

#### NamedPipeServer (named_pipe_server.h/cpp)
- **Purpose**: Listen for commands from main executable
- **Implementation**: Named pipe server at `\\.\pipe\afglobal_command`
- **Responsibilities**:
  - Accept client connections
  - Parse incoming commands
  - Update config in real-time
  - Send responses back

- **Command Handler**:
  ```
  switch(command) {
    case TOGGLE_FRAME_GEN:
      config.enabled = !config.enabled
      return SUCCESS
      
    case GET_STATUS:
      return config.enabled ? SUCCESS : FAILURE
      
    case SET_MULTIPLIER:
      if (param in [2,4]) config.multiplier = param
      return SUCCESS or INVALID_PARAM
      
    case SHUTDOWN:
      return SUCCESS (graceful exit)
  }
  ```

### Shared Components

#### ConfigManager (config.h/cpp)
- **Purpose**: Thread-safe configuration management
- **Implementation**: Singleton pattern
- **Data Source**: `afglobal_config.json`

- **Thread Safety**:
  ```cpp
  shared_mutex config_mutex_;
  
  GetConfig() {
    lock_guard lock(config_mutex_)
    return &config_;
  }
  
  UpdateConfig(newConfig) {
    lock_guard lock(config_mutex_)
    config_ = newConfig
  }
  ```

- **Config Validation**:
  - target_multiplier: 2-4 only
  - JSON parsing with error handling
  - Auto-generate defaults if missing

#### Logger (logger.h/cpp)
- **Purpose**: Structured logging to file
- **Log Levels**: DEBUG, INFO, WARNING, ERR, CRITICAL
- **Output**: `afglobal.log` with timestamps
- **Thread Safety**: Mutex-protected file writes
- **Buffering**: Line-buffered for real-time diagnostics

- **Log Format**:
  ```
  [YYYY-MM-DD HH:MM:SS] [LEVEL] Message
  ```

## Data Flow Diagrams

### Application Startup

```
main()
  ↓
TrayApp::Initialize()
  ├─→ Logger::Initialize()
  ├─→ ConfigManager::ReloadConfig()
  ├─→ TrayApp::CreateMainWindow()
  ├─→ TrayApp::SetupTrayIcon()
  ├─→ RegisterGlobalHotkey(Ctrl+Shift+F)
  ├─→ ProcessMonitor::Start()
  │    └─→ spawn monitor_thread_
  └─→ Success!
        ↓
   Message Loop (TrayApp::Run())
```

### DLL Injection Flow

```
ProcessMonitor Thread:
  Detect new game process
    ↓
  IsGameProcess() check passes
    ↓
  Callback: HandleNewProcess()
    ↓
  Injector::InjectDLL()
    ├─→ OpenProcess()
    ├─→ VirtualAllocEx()
    ├─→ WriteProcessMemory()
    ├─→ CreateRemoteThread(LoadLibraryA)
    ├─→ WaitForSingleObject()
    ├─→ VerifyInjection()
    └─→ ShowBalloonNotification()
```

### Frame Generation Flow

```
Game Present() called:
  ↓
HookedPresent() intercepted
  ├─→ GetBuffer(backbuffer)
  ├─→ FrameInterpolator::ProcessFrame()
  │    ├─→ Store frame in ring buffer
  │    ├─→ If AMF available:
  │    │    ├─→ Compute motion vectors
  │    │    └─→ Warp frames
  │    └─→ Else:
  │         └─→ Blend frames (fallback)
  │
  ├─→ For each interpolated frame (1 to multiplier-1):
  │    ├─→ Generate interpolated pixels
  │    └─→ Present()
  │
  └─→ Present() original current frame
```

## Concurrency & Thread Safety

### Thread Boundaries

1. **Main Executable**:
   - Main thread: GUI message loop
   - Monitor thread: Process enumeration
   - Pipe client: Synchronous (blocks until response)

2. **Injected DLL**:
   - Game thread: Present() calls
   - Pipe server thread: Named pipe listener
   - Optical flow thread: AMF computation pool

### Synchronization

**ProcessMonitor**:
```cpp
std::set<unsigned long> tracked_pids_;
std::mutex tracked_pids_mutex_;

// Multiple readers, single modifier
lock_guard lock(tracked_pids_mutex_);
```

**ConfigManager**:
```cpp
std::mutex config_mutex_;

// Allow multiple readers
shared_lock for reads (would use if upgradeable_mutex)
unique_lock for writes
```

**FrameInterpolator**:
```cpp
std::mutex frame_mutex_;

// Protect frame buffer access from Present() thread
lock_guard protects ProcessFrame()
```

## Memory Management

### Executable Memory Usage
- Base: ~5-10 MB
- Tracking: ~1 MB per 100 processes
- Logs: Rotating, max ~50 MB

### DLL Memory Usage
- Base: ~3-5 MB
- Frame buffers: 2 × (resolution × 4 bytes)
  - 1080p: ~16 MB
  - 4K: ~32 MB
- Motion vectors: ~1/4 of frame size

### Total Footprint
- Idle: ~50 MB
- Active at 1080p: ~100-150 MB
- Active at 4K: ~150-200 MB

## Performance Characteristics

### CPU Overhead
- Monitor thread: <1% (sleeps ~99% of time)
- Present thread: 2-5% per extra frame
- Named pipe: <0.1% (event-driven)

### GPU Overhead
- Texture copies: ~0.5 ms
- Optical flow (AMF): 1-3 ms per frame
- Blending: <0.5 ms per frame

### Latency
- Frame copy: <1 ms
- Optical flow: 1-3 ms
- Blending: <0.5 ms
- **Total latency per frame**: 1-3.5 ms

## Extension Points

### Adding New Graphics APIs

1. Modify `ProcessMonitor::IsGraphicsProcess()`
2. Add module detection for new API
3. Create new hook set (e.g., `dxgi_hooks.cpp` variant)
4. Register hooks in DLL main

### Adding Custom Frame Processing

1. Create new class in `afglobal_dll/`
2. Hook into `FrameInterpolator::ProcessFrame()`
3. Add config options in `afglobal_config.json`
4. Implement algorithm

### Custom Motion Estimation

1. Create alternative to `AMFOpticalFlow`
2. Implement same interface
3. Switch in DLL main based on config
4. Document GPU/driver requirements

## Security Considerations

### Process Injection
- Uses standard Windows API (not stealthy)
- Requires administrator privileges
- Visible in process tree tools
- Detectable by anti-cheat

### Code Injection Points
- DLL injection: Detected by most anti-cheat
- API hooking: Partially detectable
- Optical flow: Undetectable (standard GPU compute)

### Mitigation for Games
- Add to blacklist in config
- Disable for VAC, EAC, etc. protected games
- Use only in single-player mode

## Debugging

### Enable Debug Logging
```json
{
  "log_level": "DEBUG"
}
```

### Check Named Pipe Connection
```powershell
# PowerShell: Check if pipe exists
$pipe = [System.IO.Pipes.NamedPipeClientStream]::new("afglobal_command")
$pipe.Connect(1000)  # 1 second timeout
```

### Process Tree
```
explorer.exe
├── afglobal.exe (main process)
│   └── game.exe (with afglobal_hook.dll injected)
```

## Future Enhancements

1. **DirectX 9/10 Support**: Add hooks for older games
2. **Vulkan Direct Support**: Vulkan layer instead of DXGI
3. **ML-based Interpolation**: ONNX models for super-resolution
4. **Per-Game Profiles**: Save settings per game
5. **GUI Configuration**: Remove need to edit JSON
6. **Performance Monitoring**: Real-time stats overlay
7. **A.I. Training Mode**: Record game footage for ML training
8. **Network Streaming**: Cloud gaming optimization

## References

- MinHook Documentation: https://github.com/TsudaKageyu/minhook
- Windows API Reference: https://docs.microsoft.com/en-us/windows/win32/
- DXGI Reference: https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/
- AMD AMF Documentation: https://gpuopen.com/amf/
