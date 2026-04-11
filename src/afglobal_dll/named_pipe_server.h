#pragma once

#include "../common/constants.h"
#include <thread>
#include <atomic>

namespace afglobal {

class NamedPipeServer {
public:
    static NamedPipeServer& Instance();

    bool Start();
    void Stop();
    bool IsRunning() const;

private:
    NamedPipeServer() = default;
    ~NamedPipeServer() = default;

    NamedPipeServer(const NamedPipeServer&) = delete;
    NamedPipeServer& operator=(const NamedPipeServer&) = delete;

    void ServerThread();
    void HandleClient(void* pipeHandle);
    void ProcessCommand(PipeCommand cmd, int param, PipeResponse& response);

    std::thread server_thread_;
    std::atomic<bool> should_stop_{false};
    std::atomic<bool> is_running_{false};
};

}  // namespace afglobal
