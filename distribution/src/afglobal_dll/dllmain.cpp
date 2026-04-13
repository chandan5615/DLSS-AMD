#include <windows.h>
#include "../common/logger.h"
#include "../common/config.h"
#include "dxgi_hooks.h"
#include "named_pipe_server.h"
#include "frame_interpolator.h"
#include "amf_optical_flow.h"

namespace afglobal {
    extern NamedPipeServer& GetNamedPipeServer();
    extern DXGIHooks& GetDXGIHooks();
}

using namespace afglobal;

HMODULE g_DllModule = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    (void)lpReserved;

    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            g_DllModule = hModule;

            // Initialize logger
            char logPath[MAX_PATH];
            GetModuleFileNameA(nullptr, logPath, MAX_PATH);
            std::string exePath(logPath);
            size_t pos = exePath.find_last_of("\\");
            if (pos != std::string::npos) {
                std::string exeDir = exePath.substr(0, pos);
                std::string logFile = exeDir + "\\afglobal.log";
                Logger::Instance().Initialize(logFile, LogLevel::INFO);
            }

            Logger::Instance().Info("afglobal_hook.dll loaded into process");

            // Load configuration
            char modPath[MAX_PATH];
            if (GetModuleFileNameA(hModule, modPath, MAX_PATH)) {
                std::string dllDir = modPath;
                pos = dllDir.find_last_of("\\");
                if (pos != std::string::npos) {
                    dllDir = dllDir.substr(0, pos);
                    std::string configPath = dllDir + "\\afglobal_config.json";
                    ConfigManager::Instance().ReloadConfig(configPath);
                }
            }

            // Initialize AMF optical flow
            AMFOpticalFlow::Instance().Initialize();

            // Initialize DXGI hooks
            if (!DXGIHooks::Instance().Initialize()) {
                Logger::Instance().Warning("Failed to initialize DXGI hooks");
            }

            // Start named pipe server
            NamedPipeServer::Instance().Start();

            Logger::Instance().Info("afglobal_hook.dll initialization complete");
            break;
        }

        case DLL_PROCESS_DETACH: {
            Logger::Instance().Info("afglobal_hook.dll unloading from process");

            // Stop named pipe server
            NamedPipeServer::Instance().Stop();

            // Shutdown DXGI hooks
            DXGIHooks::Instance().Shutdown();

            // Shutdown AMF optical flow
            AMFOpticalFlow::Instance().Shutdown();

            // Shutdown frame interpolator
            FrameInterpolator::Instance().Shutdown();

            Logger::Instance().Info("afglobal_hook.dll unloaded successfully");
            break;
        }

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}
