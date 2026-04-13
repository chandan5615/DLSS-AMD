#include "named_pipe_server.h"
#include "../common/logger.h"
#include "../common/config.h"
#include <windows.h>
#include <thread>

namespace afglobal {

NamedPipeServer& NamedPipeServer::Instance() {
    static NamedPipeServer instance;
    return instance;
}

bool NamedPipeServer::Start() {
    if (is_running_.exchange(true)) {
        return true;  // Already running
    }

    should_stop_ = false;
    server_thread_ = std::thread(&NamedPipeServer::ServerThread, this);
    Logger::Instance().Info("Named pipe server started");
    return true;
}

void NamedPipeServer::Stop() {
    should_stop_ = true;
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
    is_running_ = false;
    Logger::Instance().Info("Named pipe server stopped");
}

bool NamedPipeServer::IsRunning() const {
    return is_running_;
}

void NamedPipeServer::ServerThread() {
    while (!should_stop_) {
        HANDLE hPipe = CreateNamedPipeA(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
            PIPE_UNLIMITED_INSTANCES,
            1024,
            1024,
            0,
            nullptr
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            Logger::Instance().Warning("Failed to create named pipe");
            Sleep(1000);
            continue;
        }

        if (ConnectNamedPipe(hPipe, nullptr)) {
            std::thread clientThread(&NamedPipeServer::HandleClient, this, hPipe);
            clientThread.detach();
        }
        else {
            CloseHandle(hPipe);
        }
    }
}

void NamedPipeServer::HandleClient(void* pipeHandle) {
    HANDLE hPipe = (HANDLE)pipeHandle;

    while (!should_stop_) {
        struct {
            int command;
            int parameter;
        } message = {};

        DWORD bytesRead;
        if (!ReadFile(hPipe, &message, sizeof(message), &bytesRead, nullptr)) {
            break;
        }

        if (bytesRead != sizeof(message)) {
            continue;
        }

        PipeCommand cmd = static_cast<PipeCommand>(message.command);
        PipeResponse response = PipeResponse::SUCCESS;

        ProcessCommand(cmd, message.parameter, response);

        DWORD bytesWritten;
        int responseCode = static_cast<int>(response);
        if (!WriteFile(hPipe, &responseCode, sizeof(responseCode), &bytesWritten, nullptr)) {
            break;
        }
    }

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
}

void NamedPipeServer::ProcessCommand(PipeCommand cmd, int param, PipeResponse& response) {
    Config config = ConfigManager::Instance().GetConfig();

    switch (cmd) {
        case PipeCommand::TOGGLE_FRAME_GEN: {
            config.enabled = !config.enabled;
            ConfigManager::Instance().UpdateConfig(config);
            Logger::Instance().Info("Frame generation toggled: " + std::string(config.enabled ? "on" : "off"));
            response = PipeResponse::SUCCESS;
            break;
        }

        case PipeCommand::GET_STATUS: {
            response = config.enabled ? PipeResponse::SUCCESS : PipeResponse::FAILURE;
            break;
        }

        case PipeCommand::SET_MULTIPLIER: {
            if (param >= 2 && param <= 4) {
                config.target_multiplier = param;
                ConfigManager::Instance().UpdateConfig(config);
                Logger::Instance().Info("Frame multiplier set to: " + std::to_string(param));
                response = PipeResponse::SUCCESS;
            }
            else {
                Logger::Instance().Warning("Invalid multiplier: " + std::to_string(param));
                response = PipeResponse::INVALID_PARAM;
            }
            break;
        }

        case PipeCommand::SHUTDOWN: {
            Logger::Instance().Info("Shutdown command received from main app");
            response = PipeResponse::SUCCESS;
            break;
        }

        default: {
            Logger::Instance().Warning("Unknown pipe command: " + std::to_string(static_cast<int>(cmd)));
            response = PipeResponse::UNKNOWN_CMD;
            break;
        }
    }
}

}  // namespace afglobal
