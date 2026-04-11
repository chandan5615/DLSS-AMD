#pragma once

#include "process_monitor.h"
#include "injector.h"
#include "named_pipe_client.h"
#include "gui_window.h"
#include <windows.h>
#include <string>
#include <memory>

namespace afglobal {

class GUIWindow;

class TrayApp {
public:
    TrayApp();
    ~TrayApp();

    bool Initialize(HINSTANCE hInstance);
    void Run();
    void Shutdown();

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    bool CreateMainWindow();
    bool SetupTrayIcon();
    void ShowTrayMenu(HWND hwnd);
    void ToggleFrameGeneration();
    void OpenConfigFile();
    void ViewLogs();
    void ShowBalloonNotification(const std::string& title, const std::string& message, bool success);
    void RegisterHotkey();
    void UnregisterHotkey();
    void HandleNewProcess(const ProcessInfo& process);

    static TrayApp* instance_;

    HWND main_window_;
    HINSTANCE hinstance_;
    HMENU tray_menu_;
    
    std::unique_ptr<ProcessMonitor> process_monitor_;
    std::unique_ptr<Injector> injector_;
    std::unique_ptr<NamedPipeClient> pipe_client_;
    std::unique_ptr<GUIWindow> gui_window_;

    bool frame_gen_enabled_;
    std::string exe_directory_;
};

}  // namespace afglobal
