#include "injector.h"
#include "../common/logger.h"
#include <windows.h>
#include <psapi.h>
#include <algorithm>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "kernel32.lib")

namespace afglobal {

Injector::Injector(const std::string& dllPath) : dll_path_(dllPath) {}

Injector::~Injector() {}

bool Injector::InjectDLL(unsigned long targetPid) {
    Logger::Instance().Info("Attempting to inject DLL into PID: " + std::to_string(targetPid));

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetPid);
    if (!hProcess) {
        Logger::Instance().Error("Failed to open target process: " + std::to_string(GetLastError()));
        return false;
    }

    // Allocate memory in the target process for the DLL path
    size_t pathSize = dll_path_.length() + 1;
    void* pRemotePath = VirtualAllocEx(hProcess, nullptr, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    
    if (!pRemotePath) {
        Logger::Instance().Error("Failed to allocate memory in target process");
        CloseHandle(hProcess);
        return false;
    }

    // Write the DLL path to the remote process
    if (!WriteProcessMemory(hProcess, pRemotePath, (void*)dll_path_.c_str(), pathSize, nullptr)) {
        Logger::Instance().Error("Failed to write DLL path to target process");
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Get address of LoadLibraryA
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    if (!hKernel32) {
        Logger::Instance().Error("Failed to get kernel32.dll handle");
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    LPVOID pLoadLibrary = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryA");
    if (!pLoadLibrary) {
        Logger::Instance().Error("Failed to get LoadLibraryA address");
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Create remote thread to load the DLL
    HANDLE hRemoteThread = CreateRemoteThread(hProcess, nullptr, 0,
                                              (LPTHREAD_START_ROUTINE)pLoadLibrary,
                                              pRemotePath, 0, nullptr);

    if (!hRemoteThread) {
        Logger::Instance().Error("Failed to create remote thread: " + std::to_string(GetLastError()));
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Wait for the thread to complete
    WaitForSingleObject(hRemoteThread, 5000);  // 5 second timeout

    // Get the thread's return value (module handle)
    DWORD threadResult = 0;
    GetExitCodeThread(hRemoteThread, &threadResult);

    CloseHandle(hRemoteThread);
    VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    if (threadResult == 0) {
        Logger::Instance().Error("LoadLibrary failed in target process");
        return false;
    }

    // Wait a bit for the DLL to initialize
    Sleep(500);

    if (VerifyInjection(targetPid)) {
        std::lock_guard<std::mutex> lock(injected_pids_mutex_);
        injected_pids_.insert(targetPid);
        Logger::Instance().Info("DLL successfully injected into PID: " + std::to_string(targetPid));
        return true;
    }

    Logger::Instance().Error("DLL injection verification failed");
    return false;
}

bool Injector::UnloadDLL(unsigned long targetPid) {
    Logger::Instance().Info("Attempting to unload DLL from PID: " + std::to_string(targetPid));

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetPid);
    if (!hProcess) {
        Logger::Instance().Error("Failed to open target process for unload");
        return false;
    }

    // Find the module
    std::string dllName = dll_path_;
    size_t pos = dllName.find_last_of("\\");
    if (pos != std::string::npos) {
        dllName = dllName.substr(pos + 1);
    }

    HMODULE hMod[256];
    DWORD cbNeeded;
    HMODULE targetModule = nullptr;

    if (EnumProcessModules(hProcess, hMod, sizeof(hMod), &cbNeeded)) {
        for (DWORD i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
            char moduleName[MAX_PATH];
            GetModuleBaseName(hProcess, hMod[i], moduleName, sizeof(moduleName));
            
            if (_stricmp(moduleName, dllName.c_str()) == 0) {
                targetModule = hMod[i];
                break;
            }
        }
    }

    if (!targetModule) {
        Logger::Instance().Warning("DLL module not found in target process");
        CloseHandle(hProcess);
        return false;
    }

    // Get address of FreeLibrary
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    LPVOID pFreeLibrary = (LPVOID)GetProcAddress(hKernel32, "FreeLibrary");

    if (!pFreeLibrary) {
        Logger::Instance().Error("Failed to get FreeLibrary address");
        CloseHandle(hProcess);
        return false;
    }

    // Create remote thread to unload the DLL
    HANDLE hRemoteThread = CreateRemoteThread(hProcess, nullptr, 0,
                                              (LPTHREAD_START_ROUTINE)pFreeLibrary,
                                              targetModule, 0, nullptr);

    if (!hRemoteThread) {
        Logger::Instance().Error("Failed to create remote thread for unload");
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hRemoteThread, 5000);
    CloseHandle(hRemoteThread);

    Sleep(500);

    if (!VerifyInjection(targetPid)) {
        std::lock_guard<std::mutex> lock(injected_pids_mutex_);
        injected_pids_.erase(targetPid);
        Logger::Instance().Info("DLL successfully unloaded from PID: " + std::to_string(targetPid));
        return true;
    }

    CloseHandle(hProcess);
    return false;
}

bool Injector::VerifyInjection(unsigned long targetPid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, targetPid);
    if (!hProcess) {
        return false;
    }

    std::string dllName = dll_path_;
    size_t pos = dllName.find_last_of("\\");
    if (pos != std::string::npos) {
        dllName = dllName.substr(pos + 1);
    }

    HMODULE hMod[256];
    DWORD cbNeeded;
    bool found = false;

    if (EnumProcessModules(hProcess, hMod, sizeof(hMod), &cbNeeded)) {
        for (DWORD i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
            char moduleName[MAX_PATH];
            GetModuleBaseName(hProcess, hMod[i], moduleName, sizeof(moduleName));
            
            if (_stricmp(moduleName, dllName.c_str()) == 0) {
                found = true;
                break;
            }
        }
    }

    CloseHandle(hProcess);
    return found;
}

std::string Injector::FindDLLInProcess(unsigned long targetPid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, targetPid);
    if (!hProcess) return "";

    std::string dllName = dll_path_;
    size_t pos = dllName.find_last_of("\\");
    if (pos != std::string::npos) {
        dllName = dllName.substr(pos + 1);
    }

    HMODULE hMod[256];
    DWORD cbNeeded;
    std::string result;

    if (EnumProcessModules(hProcess, hMod, sizeof(hMod), &cbNeeded)) {
        for (DWORD i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
            char modulePath[MAX_PATH];
            GetModuleFileNameExA(hProcess, hMod[i], modulePath, sizeof(modulePath));
            
            std::string modName = modulePath;
            size_t modPos = modName.find_last_of("\\");
            if (modPos != std::string::npos) {
                modName = modName.substr(modPos + 1);
            }

            if (_stricmp(modName.c_str(), dllName.c_str()) == 0) {
                result = modulePath;
                break;
            }
        }
    }

    CloseHandle(hProcess);
    return result;
}

}  // namespace afglobal
