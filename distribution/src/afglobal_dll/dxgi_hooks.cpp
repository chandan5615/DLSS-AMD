#include "dxgi_hooks.h"
#include "../common/logger.h"
#include "../common/config.h"
#include "frame_interpolator.h"
#include <minhook.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "minhook.lib")

namespace afglobal {

DXGIHooks& DXGIHooks::Instance() {
    static DXGIHooks instance;
    return instance;
}

namespace Hooks {
    PFN_DXGI_SWAPCHAIN_PRESENT OriginalDXGISwapChainPresent = nullptr;
    PFN_DXGI_SWAPCHAIN_PRESENT1 OriginalDXGISwapChainPresent1 = nullptr;
    PFN_CREATE_DXGI_FACTORY OriginalCreateDXGIFactory = nullptr;

    HRESULT WINAPI HookedDXGISwapChainPresent(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags) {
        const auto& config = ConfigManager::Instance().GetConfig();

        if (config.enabled) {
            // Get back buffer
            ID3D12Resource* backBuffer = nullptr;
            if (SUCCEEDED(pThis->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) {
                FrameInterpolator::Instance().ProcessFrame(backBuffer, config.target_multiplier);
                backBuffer->Release();
            }
        }

        if (OriginalDXGISwapChainPresent) {
            return OriginalDXGISwapChainPresent(pThis, SyncInterval, Flags);
        }

        return E_FAIL;
    }

    HRESULT WINAPI HookedDXGISwapChainPresent1(IDXGISwapChain1* pThis, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters) {
        const auto& config = ConfigManager::Instance().GetConfig();

        if (config.enabled) {
            // Get back buffer
            ID3D12Resource* backBuffer = nullptr;
            if (SUCCEEDED(pThis->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) {
                FrameInterpolator::Instance().ProcessFrame(backBuffer, config.target_multiplier);
                backBuffer->Release();
            }
        }

        if (OriginalDXGISwapChainPresent1) {
            return OriginalDXGISwapChainPresent1(pThis, SyncInterval, PresentFlags, pPresentParameters);
        }

        return E_FAIL;
    }

    HRESULT WINAPI HookedCreateDXGIFactory(REFIID riid, void** ppFactory) {
        // Call original
        HRESULT hr = OriginalCreateDXGIFactory(riid, ppFactory);

        if (SUCCEEDED(hr) && ppFactory) {
            // Here we could wrap the factory to spoof GPU capabilities
            // For now, just trace the call
            Logger::Instance().Debug("CreateDXGIFactory called");
        }

        return hr;
    }
}

bool DXGIHooks::Initialize() {
    if (initialized_) return true;

    if (MH_Initialize() != MH_OK) {
        Logger::Instance().Error("Failed to initialize MinHook");
        return false;
    }

    initialized_ = true;
    Logger::Instance().Info("DXGI hooks initialized");
    return true;
}

void DXGIHooks::Shutdown() {
    UnhookDXGI();

    if (MH_Uninitialize() != MH_OK) {
        Logger::Instance().Warning("Failed to uninitialize MinHook");
    }

    initialized_ = false;
}

bool DXGIHooks::HookDXGI() {
    if (!initialized_) {
        Logger::Instance().Error("DXGIHooks not initialized");
        return false;
    }

    // Hook CreateDXGIFactory
    HMODULE hDXGI = GetModuleHandleA("dxgi.dll");
    if (!hDXGI) {
        Logger::Instance().Warning("dxgi.dll not loaded yet");
        return false;
    }

    PFN_CREATE_DXGI_FACTORY pCreateDXGIFactory = (PFN_CREATE_DXGI_FACTORY)GetProcAddress(hDXGI, "CreateDXGIFactory");
    if (pCreateDXGIFactory) {
        if (MH_CreateHook(pCreateDXGIFactory, Hooks::HookedCreateDXGIFactory, (LPVOID*)&Hooks::OriginalCreateDXGIFactory) != MH_OK) {
            Logger::Instance().Error("Failed to hook CreateDXGIFactory");
            return false;
        }

        if (MH_EnableHook(pCreateDXGIFactory) != MH_OK) {
            Logger::Instance().Error("Failed to enable CreateDXGIFactory hook");
            return false;
        }
    }

    Logger::Instance().Info("DXGI hooks applied");
    return true;
}

void DXGIHooks::UnhookDXGI() {
    if (!initialized_) return;

    // Disable all hooks
    MH_DisableHook(MH_ALL_HOOKS);
    Logger::Instance().Info("DXGI hooks disabled");
}

}  // namespace afglobal
