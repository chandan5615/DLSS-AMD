#pragma once

#include "../common/constants.h"
#include <string>
#include <memory>

namespace afglobal {

class NamedPipeClient {
public:
    NamedPipeClient();
    ~NamedPipeClient();

    bool Connect(int retries = 3);
    void Disconnect();
    bool IsConnected() const;

    bool SendToggleFrameGen(bool& currentState);
    bool SendGetStatus(int& multiplier, bool& enabled);
    bool SendSetMultiplier(int multiplier);
    bool SendShutdown();

private:
    bool SendCommand(PipeCommand cmd, int param = 0);
    bool ReceiveResponse(PipeResponse& response);

    void* pipe_handle_;
    bool connected_;
};

}  // namespace afglobal
