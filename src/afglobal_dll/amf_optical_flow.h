#pragma once

#include <memory>
#include <windows.h>

namespace afglobal {

class AMFOpticalFlow {
public:
    static AMFOpticalFlow& Instance();

    bool Initialize();
    void Shutdown();
    bool IsAvailable() const;

    // Calculate optical flow between two frames
    bool ComputeOpticalFlow(void* prevFrame, void* currFrame, void* outMotionVectors);

private:
    AMFOpticalFlow() = default;
    ~AMFOpticalFlow() = default;

    AMFOpticalFlow(const AMFOpticalFlow&) = delete;
    AMFOpticalFlow& operator=(const AMFOpticalFlow&) = delete;

    HMODULE amf_module_;
    bool available_;
    bool initialized_;
};

}  // namespace afglobal
