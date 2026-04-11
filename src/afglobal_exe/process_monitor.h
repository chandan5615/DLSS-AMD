#pragma once

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <set>
#include <functional>

namespace afglobal {

struct ProcessInfo {
    unsigned long pid;
    std::string name;
    std::string path;
    bool isDirectX11 = false;
    bool isDirectX12 = false;
    bool isVulkan = false;
};

class ProcessMonitor {
public:
    ProcessMonitor();
    ~ProcessMonitor();

    void Start();
    void Stop();
    bool IsRunning() const;

    typedef std::function<void(const ProcessInfo&)> ProcessCallback;
    void SetNewProcessCallback(ProcessCallback callback);

private:
    void MonitorThreadMain();
    std::vector<ProcessInfo> EnumerateProcesses();
    bool IsGameProcess(const ProcessInfo& process);
    bool IsGraphicsProcess(const std::string& moduleName);
    std::string GetProcessPath(unsigned long pid);
    std::string GetProcessName(const std::string& path);

    std::thread monitor_thread_;
    std::atomic<bool> should_stop_{false};
    std::atomic<bool> is_running_{false};
    std::set<unsigned long> tracked_pids_;
    std::mutex tracked_pids_mutex_;
    ProcessCallback new_process_callback_;
    std::mutex callback_mutex_;
};

}  // namespace afglobal
