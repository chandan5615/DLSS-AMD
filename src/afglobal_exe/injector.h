#pragma once

#include "process_monitor.h"
#include <string>
#include <set>
#include <mutex>

namespace afglobal {

class Injector {
public:
    Injector(const std::string& dllPath);
    ~Injector();

    bool InjectDLL(unsigned long targetPid);
    bool UnloadDLL(unsigned long targetPid);
    bool VerifyInjection(unsigned long targetPid);

private:
    bool RemoteLoadLibrary(unsigned long targetPid);
    bool RemoteFreeLibrary(unsigned long targetPid);
    std::string FindDLLInProcess(unsigned long targetPid);

    std::string dll_path_;
    std::set<unsigned long> injected_pids_;
    std::mutex injected_pids_mutex_;
};

}  // namespace afglobal
