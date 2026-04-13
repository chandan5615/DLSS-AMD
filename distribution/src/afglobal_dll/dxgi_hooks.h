#pragma once

#include <d3d12.h>
#include <dxgi1_5.h>
#include <memory>

namespace afglobal {

typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY)(REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* PFN_DXGI_SWAPCHAIN_PRESENT)(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags);
typedef HRESULT(WINAPI* PFN_DXGI_SWAPCHAIN_PRESENT1)(IDXGISwapChain1* pThis, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters);

class DXGIHooks {
public:
    static DXGIHooks& Instance();

    bool Initialize();
    void Shutdown();

    bool HookDXGI();
    void UnhookDXGI();

private:
    DXGIHooks() = default;
    ~DXGIHooks() = default;

    DXGIHooks(const DXGIHooks&) = delete;
    DXGIHooks& operator=(const DXGIHooks&) = delete;

    bool initialized_;
};

// Hot function prototypes for D3D interfaces
namespace Hooks {
    extern PFN_DXGI_SWAPCHAIN_PRESENT OriginalDXGISwapChainPresent;
    extern PFN_DXGI_SWAPCHAIN_PRESENT1 OriginalDXGISwapChainPresent1;
    extern PFN_CREATE_DXGI_FACTORY OriginalCreateDXGIFactory;

    HRESULT WINAPI HookedDXGISwapChainPresent(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags);
    HRESULT WINAPI HookedDXGISwapChainPresent1(IDXGISwapChain1* pThis, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters);
    HRESULT WINAPI HookedCreateDXGIFactory(REFIID riid, void** ppFactory);
}

}  // namespace afglobal
