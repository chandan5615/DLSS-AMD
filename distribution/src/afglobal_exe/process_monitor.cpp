#include "process_monitor.h"
#include "../common/logger.h"
#include "../common/config.h"
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <algorithm>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "kernel32.lib")

namespace afglobal {

ProcessMonitor::ProcessMonitor() : should_stop_(false), is_running_(false) {}

ProcessMonitor::~ProcessMonitor() {
    Stop();
}

void ProcessMonitor::Start() {
    if (is_running_.exchange(true)) {
        return;  // Already running
    }

    should_stop_ = false;
    monitor_thread_ = std::thread(&ProcessMonitor::MonitorThreadMain, this);
    Logger::Instance().Info("Process monitor started");
}

void ProcessMonitor::Stop() {
    should_stop_ = true;
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
    is_running_ = false;
    Logger::Instance().Info("Process monitor stopped");
}

bool ProcessMonitor::IsRunning() const {
    return is_running_;
}

void ProcessMonitor::SetNewProcessCallback(ProcessCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    new_process_callback_ = callback;
}

void ProcessMonitor::MonitorThreadMain() {
    while (!should_stop_) {
        try {
            auto processes = EnumerateProcesses();
            
            for (const auto& process : processes) {
                bool isNew = false;
                {
                    std::lock_guard<std::mutex> lock(tracked_pids_mutex_);
                    if (tracked_pids_.find(process.pid) == tracked_pids_.end()) {
                        tracked_pids_.insert(process.pid);
                        isNew = true;
                    }
                }

                if (isNew && IsGameProcess(process)) {
                    Logger::Instance().Info("Detected new game process: " + process.name + " (PID: " + std::to_string(process.pid) + ")");
                    
                    {
                        std::lock_guard<std::mutex> lock(callback_mutex_);
                        if (new_process_callback_) {
                            new_process_callback_(process);
                        }
                    }
                }
            }

            // Clean up tracking for dead processes
            {
                std::lock_guard<std::mutex> lock(tracked_pids_mutex_);
                for (auto it = tracked_pids_.begin(); it != tracked_pids_.end(); ) {
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, *it);
                    if (!hProcess) {
                        it = tracked_pids_.erase(it);
                    } else {
                        DWORD exitCode;
                        if (!GetExitCodeProcess(hProcess, &exitCode) || exitCode != STILL_ACTIVE) {
                            it = tracked_pids_.erase(it);
                        } else {
                            ++it;
                        }
                        CloseHandle(hProcess);
                    }
                }
            }
        }
        catch (const std::exception& e) {
            Logger::Instance().Error("Process monitor error: " + std::string(e.what()));
        }

        Sleep(PROCESS_MONITOR_INTERVAL_MS);
    }
}

std::vector<ProcessInfo> ProcessMonitor::EnumerateProcesses() {
    std::vector<ProcessInfo> result;
    DWORD processIds[PROCESS_ENUM_BATCH_SIZE];
    DWORD bytesReturned = 0;

    if (!EnumProcesses(processIds, sizeof(processIds), &bytesReturned)) {
        Logger::Instance().Warning("EnumProcesses failed");
        return result;
    }

    DWORD processCount = bytesReturned / sizeof(DWORD);

    for (DWORD i = 0; i < processCount; ++i) {
        DWORD pid = processIds[i];
        if (pid == 0) continue;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (!hProcess) continue;

        HMODULE hMod[256];
        DWORD cbNeeded;
        auto modulePath = GetProcessPath(pid);
        
        if (!modulePath.empty()) {
            ProcessInfo info;
            info.pid = pid;
            info.path = modulePath;
            info.name = GetProcessName(modulePath);

            // Check for graphics APIs
            if (EnumProcessModules(hProcess, hMod, sizeof(hMod), &cbNeeded)) {
                for (DWORD j = 0; j < (cbNeeded / sizeof(HMODULE)); ++j) {
                    char moduleName[MAX_PATH];
                    GetModuleBaseName(hProcess, hMod[j], moduleName, sizeof(moduleName));
                    
                    std::string modNameLower = moduleName;
                    std::transform(modNameLower.begin(), modNameLower.end(), modNameLower.begin(), ::tolower);

                    if (modNameLower.find("d3d11") != std::string::npos) {
                        info.isDirectX11 = true;
                    }
                    if (modNameLower.find("d3d12") != std::string::npos ||
                        modNameLower.find("dxgi") != std::string::npos) {
                        info.isDirectX12 = true;
                    }
                    if (modNameLower.find("vulkan") != std::string::npos) {
                        info.isVulkan = true;
                    }
                }
            }

            result.push_back(info);
        }

        CloseHandle(hProcess);
    }

    return result;
}

bool ProcessMonitor::IsGameProcess(const ProcessInfo& process) {
    const auto& config = ConfigManager::Instance().GetConfig();

    // Check blacklist first
    std::string lowerName = process.name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

    if (ConfigManager::Instance().IsProcessBlacklisted(lowerName)) {
        return false;
    }

    // Check whitelist
    if (!config.process_whitelist.empty()) {
        if (!ConfigManager::Instance().IsProcessWhitelisted(lowerName)) {
            return false;
        }
    }

    // Must have at least one graphics API
    return (process.isDirectX11 || process.isDirectX12 || process.isVulkan);
}

std::string ProcessMonitor::GetProcessPath(unsigned long pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return "";

    char path[MAX_PATH];
    DWORD size = sizeof(path);
    
    if (QueryFullProcessImageNameA(hProcess, 0, path, &size)) {
        CloseHandle(hProcess);
        return std::string(path);
    }

    CloseHandle(hProcess);
    return "";
}

std::string ProcessMonitor::GetProcessName(const std::string& path) {
    size_t lastSlash = path.find_last_of("\\");
    if (lastSlash != std::string::npos) {
        return path.substr(lastSlash + 1);
    }
    return path;
}

}  // namespace afglobal
