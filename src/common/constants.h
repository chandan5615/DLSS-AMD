#pragma once

#include <string>
#include <windows.h>

namespace afglobal {

// Application constants
constexpr const char* APP_NAME = "AMD Global Frame Generator";
constexpr const char* APP_VERSION = "1.0.0";
constexpr const char* APP_COMPANY = "Advanced Micro Devices";
constexpr const char* DLL_NAME = "afglobal_hook.dll";
constexpr const char* CONFIG_FILE = "afglobal_config.json";
constexpr const char* LOG_FILE = "afglobal.log";

// Named pipe constants
constexpr const char* PIPE_NAME = "\\\\.\\pipe\\afglobal_command";

// Process monitoring constants
constexpr int PROCESS_MONITOR_INTERVAL_MS = 1000;  // Check for new processes every 1 second
constexpr int PROCESS_ENUM_BATCH_SIZE = 512;       // Max processes to enumerate at once

// Frame interpolation constants
constexpr int FRAME_BUFFER_SIZE = 2;  // Store last 2 frames
constexpr int MIN_FRAME_MULTIPLIER = 2;
constexpr int MAX_FRAME_MULTIPLIER = 4;

// Window class and messages (moved to tray_app.cpp to avoid preprocessor conflicts)
constexpr const char* MAIN_WINDOW_CLASS = "AFGlobalMainWindow";
// WM_TRAY_ICON, WM_HOTKEY, HOTKEY_ID defined in tray_app.cpp

// Log levels
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERR,
    CRITICAL
};

// Named pipe commands
enum class PipeCommand : int {
    TOGGLE_FRAME_GEN = 1,
    GET_STATUS = 2,
    SET_MULTIPLIER = 3,
    SHUTDOWN = 4
};

// Named pipe response codes
enum class PipeResponse : int {
    SUCCESS = 0,
    FAILURE = 1,
    UNKNOWN_CMD = 2,
    INVALID_PARAM = 3
};

}  // namespace afglobal
