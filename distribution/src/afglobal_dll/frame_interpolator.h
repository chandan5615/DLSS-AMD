#pragma once

#include <d3d12.h>
#include <memory>
#include <mutex>

namespace afglobal {

class FrameInterpolator {
public:
    static FrameInterpolator& Instance();

    void Initialize(ID3D12Device* device, ID3D12CommandQueue* queue);
    void Shutdown();

    void ProcessFrame(ID3D12Resource* currentFrame, int multiplier);

private:
    FrameInterpolator() = default;
    ~FrameInterpolator() = default;

    FrameInterpolator(const FrameInterpolator&) = delete;
    FrameInterpolator& operator=(const FrameInterpolator&) = delete;

    bool CreateFrameBuffer(ID3D12Resource* sourceFrame);
    ID3D12Resource* InterpolateFrame(ID3D12Resource* prevFrame, ID3D12Resource* currFrame);
    ID3D12Resource* BlendFrames(ID3D12Resource* prevFrame, ID3D12Resource* currFrame, float weight);

    ID3D12Device* device_;
    ID3D12CommandQueue* command_queue_;
    ID3D12Resource* frame_buffer_[2];  // Ring buffer for last 2 frames
    int buffer_index_;
    bool initialized_;
    std::mutex frame_mutex_;
};

}  // namespace afglobal
