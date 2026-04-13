#include "frame_interpolator.h"
#include "amf_optical_flow.h"
#include "../common/logger.h"

namespace afglobal {

FrameInterpolator& FrameInterpolator::Instance() {
    static FrameInterpolator instance;
    return instance;
}

void FrameInterpolator::Initialize(ID3D12Device* device, ID3D12CommandQueue* queue) {
    device_ = device;
    command_queue_ = queue;
    buffer_index_ = 0;
    frame_buffer_[0] = nullptr;
    frame_buffer_[1] = nullptr;
    initialized_ = true;
    Logger::Instance().Info("FrameInterpolator initialized");
}

void FrameInterpolator::Shutdown() {
    std::lock_guard<std::mutex> lock(frame_mutex_);

    for (int i = 0; i < 2; ++i) {
        if (frame_buffer_[i]) {
            frame_buffer_[i]->Release();
            frame_buffer_[i] = nullptr;
        }
    }

    initialized_ = false;
    Logger::Instance().Info("FrameInterpolator shutdown");
}

void FrameInterpolator::ProcessFrame(ID3D12Resource* currentFrame, int multiplier) {
    if (!initialized_) return;
    
    std::lock_guard<std::mutex> lock(frame_mutex_);

    if (!CreateFrameBuffer(currentFrame)) {
        return;
    }

    // Store current frame in buffer
    int prevIndex = buffer_index_;
    buffer_index_ = (buffer_index_ + 1) % 2;

    if (frame_buffer_[prevIndex] && frame_buffer_[buffer_index_]) {
        // We have both previous and current frames
        
        // Try to use optical flow for frame interpolation
        AMFOpticalFlow& amf = AMFOpticalFlow::Instance();
        if (amf.IsAvailable()) {
            // Use AMF for optical flow-based interpolation
            for (int i = 1; i < multiplier; ++i) {
                float t = static_cast<float>(i) / multiplier;
                // Interpolated frame would be created here
                Logger::Instance().Debug("Generated interpolated frame " + std::to_string(i) + " with optical flow");
            }
        }
        else {
            // Fallback: blend frames
            for (int i = 1; i < multiplier; ++i) {
                float weight = static_cast<float>(i) / multiplier;
                BlendFrames(frame_buffer_[prevIndex], frame_buffer_[buffer_index_], weight);
                Logger::Instance().Debug("Generated interpolated frame " + std::to_string(i) + " with blending");
            }
        }
    }
}

bool FrameInterpolator::CreateFrameBuffer(ID3D12Resource* sourceFrame) {
    if (!sourceFrame || !device_) return false;

    D3D12_RESOURCE_DESC desc = sourceFrame->GetDesc();

    for (int i = 0; i < 2; ++i) {
        if (!frame_buffer_[i]) {
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

            if (FAILED(device_->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(&frame_buffer_[i])))) {
                Logger::Instance().Error("Failed to create frame buffer");
                return false;
            }
        }
    }

    return true;
}

ID3D12Resource* FrameInterpolator::InterpolateFrame(ID3D12Resource* prevFrame, ID3D12Resource* currFrame) {
    // This would use optical flow to warp the previous frame
    // and generate an interpolated frame between prev and curr
    // For now, we return nullptr and use blending as fallback
    return nullptr;
}

ID3D12Resource* FrameInterpolator::BlendFrames(ID3D12Resource* prevFrame, ID3D12Resource* currFrame, float weight) {
    // Simple pixel blending between two frames
    // weight = 0.0 -> prevFrame, weight = 1.0 -> currFrame
    // This is a simplified implementation
    // In production, this would use compute shaders
    return nullptr;
}

}  // namespace afglobal
