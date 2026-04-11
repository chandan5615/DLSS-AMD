#include "named_pipe_client.h"
#include "../common/logger.h"
#include <windows.h>
#include <thread>
#include <chrono>

namespace afglobal {

NamedPipeClient::NamedPipeClient() : pipe_handle_(nullptr), connected_(false) {}

NamedPipeClient::~NamedPipeClient() {
    Disconnect();
}

bool NamedPipeClient::Connect(int retries) {
    for (int i = 0; i < retries; ++i) {
        HANDLE hPipe = CreateFileA(
            PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
        );

        if (hPipe != INVALID_HANDLE_VALUE) {
            pipe_handle_ = hPipe;
            connected_ = true;
            Logger::Instance().Debug("Connected to named pipe");
            return true;
        }

        if (i < retries - 1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    Logger::Instance().Warning("Failed to connect to named pipe after " + std::to_string(retries) + " retries");
    return false;
}

void NamedPipeClient::Disconnect() {
    if (pipe_handle_ && pipe_handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle((HANDLE)pipe_handle_);
        pipe_handle_ = nullptr;
    }
    connected_ = false;
}

bool NamedPipeClient::IsConnected() const {
    return connected_;
}

bool NamedPipeClient::SendToggleFrameGen(bool& currentState) {
    if (!SendCommand(PipeCommand::TOGGLE_FRAME_GEN)) {
        return false;
    }

    PipeResponse response;
    if (!ReceiveResponse(response)) {
        return false;
    }

    // The response parameter contains the new state
    currentState = (response == PipeResponse::SUCCESS);
    return true;
}

bool NamedPipeClient::SendGetStatus(int& multiplier, bool& enabled) {
    if (!SendCommand(PipeCommand::GET_STATUS)) {
        return false;
    }

    // Receive two responses: enabled status and multiplier
    PipeResponse response1, response2;
    if (!ReceiveResponse(response1)) {
        return false;
    }

    enabled = (response1 == PipeResponse::SUCCESS);
    
    // For multiplier, we'd need to extended protocol - simplified for now
    multiplier = 2;
    return true;
}

bool NamedPipeClient::SendSetMultiplier(int multiplier) {
    if (multiplier < 2 || multiplier > 4) {
        Logger::Instance().Warning("Invalid multiplier: " + std::to_string(multiplier));
        return false;
    }

    return SendCommand(PipeCommand::SET_MULTIPLIER, multiplier);
}

bool NamedPipeClient::SendShutdown() {
    return SendCommand(PipeCommand::SHUTDOWN);
}

bool NamedPipeClient::SendCommand(PipeCommand cmd, int param) {
    if (!connected_) {
        Logger::Instance().Warning("Pipe not connected");
        return false;
    }

    struct {
        int command;
        int parameter;
    } message = { static_cast<int>(cmd), param };

    DWORD bytesWritten;
    if (!WriteFile(pipe_handle_, &message, sizeof(message), &bytesWritten, nullptr)) {
        Logger::Instance().Error("Failed to write to named pipe");
        return false;
    }

    return true;
}

bool NamedPipeClient::ReceiveResponse(PipeResponse& response) {
    if (!connected_) {
        return false;
    }

    int responseCode;
    DWORD bytesRead;
    
    if (!ReadFile(pipe_handle_, &responseCode, sizeof(responseCode), &bytesRead, nullptr)) {
        Logger::Instance().Error("Failed to read from named pipe");
        return false;
    }

    if (bytesRead != sizeof(responseCode)) {
        Logger::Instance().Error("Incomplete response from named pipe");
        return false;
    }

    response = static_cast<PipeResponse>(responseCode);
    return true;
}

}  // namespace afglobal
