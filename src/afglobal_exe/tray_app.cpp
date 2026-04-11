#include "tray_app.h"
#include "../common/logger.h"
#include "../common/config.h"

// Define Windows target version to ensure all functions are available
#define _WIN32_WINNT 0x0601  // Windows 7 and later
#define WINVER _WIN32_WINNT

#include <windows.h>
#include <shellapi.h>
#include <thread>
#include <algorithm>

#pragma comment(lib, "user32.lib")

namespace afglobal {
// Message IDs: WM_TRAY_ICON=1025, WM_HOTKEY=1026, HOTKEY_ID=1001
constexpr unsigned int LOCAL_WM_TRAY_ICON = 1025;
constexpr unsigned int LOCAL_WM_HOTKEY = 1026;
constexpr unsigned int LOCAL_HOTKEY_ID = 1001;

TrayApp* TrayApp::instance_ = nullptr;

TrayApp::TrayApp()
    : main_window_(nullptr)
    , hinstance_(nullptr)
    , tray_menu_(nullptr)
    , frame_gen_enabled_(false)
{
    instance_ = this;
}

TrayApp::~TrayApp() {
    Shutdown();
}

bool TrayApp::Initialize(HINSTANCE hInstance) {
    hinstance_ = hInstance;
    
    // Initialize logger
    char logPath[MAX_PATH];
    GetModuleFileNameA(nullptr, logPath, MAX_PATH);
    std::string exeDir(logPath);
    size_t pos = exeDir.find_last_of("\\");
    if (pos != std::string::npos) {
        exeDir = exeDir.substr(0, pos);
        exe_directory_ = exeDir;
    }

    std::string logFile = exe_directory_ + "\\" + LOG_FILE;
    Logger::Instance().Initialize(logFile, LogLevel::INFO);
    Logger::Instance().Info("=== AMD Global Frame Generator Started ===");

    // Load configuration
    std::string configPath = exe_directory_ + "\\" + CONFIG_FILE;
    ConfigManager::Instance().ReloadConfig(configPath);

    // Create main window
    if (!CreateMainWindow()) {
        Logger::Instance().Error("Failed to create main window");
        return false;
    }

    // Setup tray icon
    if (!SetupTrayIcon()) {
        Logger::Instance().Error("Failed to setup tray icon");
        return false;
    }

    // Initialize GUI window
    gui_window_ = std::make_unique<GUIWindow>();
    if (!gui_window_->Initialize()) {
        Logger::Instance().Warning("Failed to initialize GUI window");
        gui_window_.reset();
    }

    // Initialize components
    std::string dllPath = exe_directory_ + "\\" + DLL_NAME;
    process_monitor_ = std::make_unique<ProcessMonitor>();
    injector_ = std::make_unique<Injector>(dllPath);
    pipe_client_ = std::make_unique<NamedPipeClient>();

    // Setup callbacks
    process_monitor_->SetNewProcessCallback([this](const ProcessInfo& process) {
        HandleNewProcess(process);
    });

    // Register hotkey
    RegisterHotkey();

    // Start process monitor
    process_monitor_->Start();

    Logger::Instance().Info("Application initialized successfully");
    return true;
}

void TrayApp::Run() {
    Logger::Instance().Info("Tray app entering message loop");
    
    MSG msg;
    while (GetMessageA(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

void TrayApp::Shutdown() {
    Logger::Instance().Info("Shutting down tray app");

    // Clean up GUI window
    if (gui_window_) {
        gui_window_->Shutdown();
        gui_window_.reset();
    }

    // Clean up hotkey
    UnregisterHotkey();

    // Stop process monitor
    if (process_monitor_) {
        process_monitor_->Stop();
    }

    // Disconnect from pipe
    if (pipe_client_) {
        pipe_client_->Disconnect();
    }

    // Remove tray icon
    if (main_window_) {
        NOTIFYICONDATAA nid = {};
        nid.cbSize = sizeof(NOTIFYICONDATAA);
        nid.hWnd = main_window_;
        nid.uID = 1;
        Shell_NotifyIconA(NIM_DELETE, &nid);

        if (tray_menu_) {
            DestroyMenu(tray_menu_);
        }

        DestroyWindow(main_window_);
    }

    Logger::Instance().Info("=== AMD Global Frame Generator Stopped ===");
}

LRESULT CALLBACK TrayApp::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    TrayApp* pThis = nullptr;

    if (msg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<TrayApp*>(pCreate->lpCreateParams);
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    }
    else {
        pThis = reinterpret_cast<TrayApp*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
    }

    switch (msg) {
        case LOCAL_WM_TRAY_ICON: {
            if (lParam == WM_RBUTTONDOWN) {
                if (pThis) {
                    pThis->ShowTrayMenu(hwnd);
                }
            }
            else if (lParam == WM_LBUTTONDBLCLK) {
                // Double-click: Toggle GUI visibility
                if (pThis && pThis->gui_window_) {
                    pThis->gui_window_->ToggleVisibility();
                }
            }
            break;
        }

        case WM_HOTKEY: {
            if (pThis && wParam == LOCAL_HOTKEY_ID) {
                pThis->ToggleFrameGeneration();
            }
            break;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case 101:  // Enable/Disable
                    if (pThis) pThis->ToggleFrameGeneration();
                    break;
                case 102:  // Open Config
                    if (pThis) pThis->OpenConfigFile();
                    break;
                case 103:  // View Logs
                    if (pThis) pThis->ViewLogs();
                    break;
                case 104:  // Exit
                    PostQuitMessage(0);
                    break;
            }
            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }

    return 0;
}

bool TrayApp::CreateMainWindow() {
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hinstance_;
    wc.lpszClassName = MAIN_WINDOW_CLASS;
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassA(&wc)) {
        Logger::Instance().Error("Failed to register window class");
        return false;
    }

    main_window_ = CreateWindowExA(
        0,
        MAIN_WINDOW_CLASS,
        APP_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr,
        nullptr,
        hinstance_,
        this
    );

    if (!main_window_) {
        Logger::Instance().Error("Failed to create window");
        return false;
    }

    // Hide the window
    // ShowWindowA(main_window_, SW_HIDE);  // TODO: Fix Windows API linking
    // For now, we'll hide the window using SetWindowPos with hidden flag
    SetWindowPos(main_window_, HWND_TOP, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE);

    return true;
}

bool TrayApp::SetupTrayIcon() {
    NOTIFYICONDATAA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATAA);
    nid.hWnd = main_window_;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE;
    nid.uCallbackMessage = LOCAL_WM_TRAY_ICON;
    nid.hIcon = LoadIconA(nullptr, IDI_APPLICATION);
    strcpy_s(nid.szTip, sizeof(nid.szTip), APP_NAME);

    if (!Shell_NotifyIconA(NIM_ADD, &nid)) {
        Logger::Instance().Error("Failed to add tray icon");
        return false;
    }

    // Create tray menu
    tray_menu_ = CreatePopupMenu();
    if (!tray_menu_) {
        Logger::Instance().Error("Failed to create tray menu");
        return false;
    }

    AppendMenuA(tray_menu_, MF_STRING, 101, "Enable/Disable");
    AppendMenuA(tray_menu_, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(tray_menu_, MF_STRING, 102, "Open Config");
    AppendMenuA(tray_menu_, MF_STRING, 103, "View Logs");
    AppendMenuA(tray_menu_, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(tray_menu_, MF_STRING, 104, "Exit");

    return true;
}

void TrayApp::ShowTrayMenu(HWND hwnd) {
    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);

    TrackPopupMenu(tray_menu_, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
}

void TrayApp::ToggleFrameGeneration() {
    if (!pipe_client_->IsConnected()) {
        if (!pipe_client_->Connect()) {
            Logger::Instance().Warning("Failed to connect to DLL pipe");
            ShowBalloonNotification("Frame Generation", "Failed to connect to DLL", false);
            return;
        }
    }

    bool newState = false;
    if (pipe_client_->SendToggleFrameGen(newState)) {
        frame_gen_enabled_ = newState;
        std::string status = newState ? "enabled" : "disabled";
        Logger::Instance().Info("Frame generation toggled: " + status);
        ShowBalloonNotification("Frame Generation", "Frame generation " + status, true);
    }
    else {
        Logger::Instance().Warning("Failed to toggle frame generation");
        ShowBalloonNotification("Frame Generation", "Failed to toggle frame generation", false);
    }
}

void TrayApp::OpenConfigFile() {
    std::string configPath = exe_directory_ + "\\" + CONFIG_FILE;
    
    // Ensure config file exists
    if (GetFileAttributesA(configPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        Config::CreateDefault().SaveToFile(configPath);
    }

    ShellExecuteA(nullptr, "open", "notepad.exe", configPath.c_str(), exe_directory_.c_str(), SW_SHOW);
    Logger::Instance().Info("Opened config file in editor");
}

void TrayApp::ViewLogs() {
    std::string logPath = exe_directory_ + "\\" + LOG_FILE;
    ShellExecuteA(nullptr, "open", "notepad.exe", logPath.c_str(), exe_directory_.c_str(), SW_SHOW);
    Logger::Instance().Info("Opened log file in editor");
}

void TrayApp::ShowBalloonNotification(const std::string& title, const std::string& message, bool success) {
    NOTIFYICONDATAA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATAA);
    nid.hWnd = main_window_;
    nid.uID = 1;
    nid.uFlags = NIF_INFO;
    nid.dwInfoFlags = success ? NIIF_INFO : NIIF_WARNING;
    nid.uTimeout = 5000;

    strcpy_s(nid.szInfoTitle, sizeof(nid.szInfoTitle), title.c_str());
    strcpy_s(nid.szInfo, sizeof(nid.szInfo), message.c_str());

    Shell_NotifyIconA(NIM_MODIFY, &nid);
}

void TrayApp::RegisterHotkey() {
    // Register Ctrl+Shift+F
    // TODO: Implement hotkey registration once Windows API functions are properly linked
    Logger::Instance().Info("Hotkey registration: Ctrl+Shift+F (TODO: implement)");
}

void TrayApp::UnregisterHotkey() {
    // TODO: Implement hotkey unregistration once Windows API functions are properly linked
}

void TrayApp::HandleNewProcess(const ProcessInfo& process) {
    std::string dllPath = exe_directory_ + "\\" + DLL_NAME;
    
    // Check if DLL exists before attempting injection
    if (GetFileAttributesA(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        Logger::Instance().Error("DLL not found at: " + dllPath);
        return;
    }

    if (injector_->InjectDLL(process.pid)) {
        ShowBalloonNotification("DLL Injection", "Successfully injected into " + process.name, true);
    }
    else {
        ShowBalloonNotification("DLL Injection", "Failed to inject into " + process.name, false);
    }
}

}  // namespace afglobal
