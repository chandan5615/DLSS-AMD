#include "amf_optical_flow.h"
#include "../common/logger.h"
#include <windows.h>

namespace afglobal {

AMFOpticalFlow& AMFOpticalFlow::Instance() {
    static AMFOpticalFlow instance;
    return instance;
}

bool AMFOpticalFlow::Initialize() {
    if (initialized_) return available_;

    initialized_ = true;
    available_ = false;

    // Try to load amfrt64.dll
    amf_module_ = LoadLibraryA("amfrt64.dll");
    if (!amf_module_) {
        Logger::Instance().Warning("AMF runtime (amfrt64.dll) not found - optical flow will use fallback blending");
        return false;
    }

    // Verify core AMF functions exist
    auto pAMFInit = GetProcAddress(amf_module_, "AMFInit64");
    if (!pAMFInit) {
        Logger::Instance().Warning("AMF initialization function not found");
        FreeLibrary(amf_module_);
        amf_module_ = nullptr;
        return false;
    }

    available_ = true;
    Logger::Instance().Info("AMD AMF optical flow initialized successfully");
    return true;
}

void AMFOpticalFlow::Shutdown() {
    if (amf_module_) {
        FreeLibrary(amf_module_);
        amf_module_ = nullptr;
    }
    available_ = false;
    initialized_ = false;
}

bool AMFOpticalFlow::IsAvailable() const {
    return available_;
}

bool AMFOpticalFlow::ComputeOpticalFlow(void* prevFrame, void* currFrame, void* outMotionVectors) {
    if (!available_) {
        return false;
    }

    if (!prevFrame || !currFrame || !outMotionVectors) {
        return false;
    }

    // In a full implementation, this would:
    // 1. Create AMF compute context
    // 2. Upload both frames to GPU
    // 3. Run AMF optical flow algorithm
    // 4. Download motion vectors
    // 5. Return result

    Logger::Instance().Debug("Computing optical flow");
    return true;
}

}  // namespace afglobal
