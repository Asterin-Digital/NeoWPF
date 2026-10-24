#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dcomp.h>
#include <wrl.h>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <stdexcept>
#include <Windows.h>

using Microsoft::WRL::ComPtr;
using namespace NeoWPF::Rendering;

namespace NeoWPF::Rendering::D3D11 {
    extern ComPtr<ID3D11Device> g_device;
    extern ComPtr<ID3D11DeviceContext> g_context;
    HRESULT CreateSwapChainForWindow(HWND hwnd, unsigned int width, unsigned int height, IDXGISwapChain** swapChain);
}

namespace {

struct RendererWindowContext {
    HWND hwnd = nullptr;
    ComPtr<IDXGISwapChain> swapChain;
    ComPtr<ID3D11RenderTargetView> rtv;
    unsigned int width = 0;
    unsigned int height = 0;
    bool transparent = false;
    ComPtr<IDCompositionTarget> compositionTarget;
    ComPtr<IDCompositionVisual> compositionVisual;
};

std::unordered_map<HWND, std::shared_ptr<RendererWindowContext>> g_windowContexts;
std::mutex g_windowContextsMutex;

std::shared_ptr<RendererWindowContext> FindContext(HWND hwnd)
{
    if (!hwnd) return nullptr;
    std::lock_guard<std::mutex> lock(g_windowContextsMutex);
    auto it = g_windowContexts.find(hwnd);
    return it == g_windowContexts.end() ? nullptr : it->second;
}

HRESULT AttachComposition(HWND hwnd, IDXGISwapChain* swapChain,
    ComPtr<IDCompositionTarget>& target, ComPtr<IDCompositionVisual>& visual)
{
    if (!hwnd || !swapChain) return E_INVALIDARG;

    ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = NeoWPF::Rendering::D3D11::g_device.As(&dxgiDevice);
    if (FAILED(hr)) return hr;

    ComPtr<IDCompositionDevice> compositionDevice;
    hr = DCompositionCreateDevice(dxgiDevice.Get(), IID_PPV_ARGS(&compositionDevice));
    if (FAILED(hr)) return hr;
    hr = compositionDevice->CreateTargetForHwnd(hwnd, TRUE, &target);
    if (FAILED(hr)) return hr;
    hr = compositionDevice->CreateVisual(&visual);
    if (FAILED(hr)) return hr;
    hr = visual->SetContent(swapChain);
    if (FAILED(hr)) return hr;
    hr = target->SetRoot(visual.Get());
    if (FAILED(hr)) return hr;
    return compositionDevice->Commit();
}

bool AttachCompositionIfNeeded(const std::shared_ptr<RendererWindowContext>& ctx,
    IDXGISwapChain* swapChain)
{
    if (!ctx || !ctx->transparent) {
        return true;
    }

    ctx->compositionTarget.Reset();
    ctx->compositionVisual.Reset();
    return SUCCEEDED(AttachComposition(
        ctx->hwnd,
        swapChain,
        ctx->compositionTarget,
        ctx->compositionVisual));
}

}

namespace NeoWPF::Rendering::D3D11 {

void InitializeRendererWindowing(HWND hwnd)
{
    if (!hwnd) return;
    if (FindContext(hwnd)) {
        return;
    }

    RECT rc;
    GetClientRect(hwnd, &rc);
    unsigned int width = rc.right - rc.left;
    unsigned int height = rc.bottom - rc.top;
    if (width == 0 || height == 0) {
        width = 1;
        height = 1;
    }

    if (!g_device) {
        throw std::runtime_error("D3D11 device must be initialized before creating a surface");
    }
    ComPtr<IDXGISwapChain> swapChain;
    HRESULT hr = CreateSwapChainForWindow(hwnd, width, height, &swapChain);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create swap chain for window");
    }

    ComPtr<ID3D11Texture2D> backBuffer;
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to get back buffer");
    }

    auto ctx = std::make_shared<RendererWindowContext>();
    ctx->hwnd = hwnd;
    ctx->swapChain = swapChain;
    ctx->width = width;
    ctx->height = height;
    ctx->transparent = ::GetPropW(hwnd, L"NeoWPF.Transparent") != nullptr;

    hr = g_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &ctx->rtv);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create render target view");
    }

    if (ctx->transparent) {
        hr = AttachComposition(hwnd, swapChain.Get(), ctx->compositionTarget, ctx->compositionVisual);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to attach transparent composition swapchain");
        }
    }

    {
        std::lock_guard<std::mutex> lock(g_windowContextsMutex);
        g_windowContexts[hwnd] = ctx;
    }
}

void ShutdownRendererWindowing()
{
    std::lock_guard<std::mutex> lock(g_windowContextsMutex);
    for (auto& pair : g_windowContexts) {
        pair.second->rtv.Reset();
        pair.second->swapChain.Reset();
    }
    g_windowContexts.clear();
}

void DestroyRendererWindowing(HWND hwnd) noexcept
{
    auto context = FindContext(hwnd);
    if (!context) {
        return;
    }
    if (NeoWPF::Rendering::D3D11::g_context) {
        NeoWPF::Rendering::D3D11::g_context->OMSetRenderTargets(0, nullptr, nullptr);
        NeoWPF::Rendering::D3D11::g_context->Flush();
    }
    std::lock_guard<std::mutex> lock(g_windowContextsMutex);
    g_windowContexts.erase(hwnd);
}

void ResizeRendererWindowing(HWND hwnd, unsigned int w, unsigned int h)
{
    auto ctx = FindContext(hwnd);
    if (!ctx) return;

    if (w == 0) w = 1;
    if (h == 0) h = 1;

    if (!ctx->swapChain) {
        ComPtr<IDXGISwapChain> rebuiltSwapChain;
        HRESULT hr = CreateSwapChainForWindow(ctx->hwnd, w, h, &rebuiltSwapChain);
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to create swap chain for window during resize (0x%08X)", static_cast<unsigned int>(hr));
            return;
        }

        ComPtr<ID3D11Texture2D> backBuffer;
        hr = rebuiltSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to get back buffer during resize (0x%08X)", static_cast<unsigned int>(hr));
            return;
        }

        ComPtr<ID3D11RenderTargetView> rebuiltRTV;
        hr = g_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &rebuiltRTV);
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to create render target view during resize (0x%08X)", static_cast<unsigned int>(hr));
            return;
        }

        ctx->rtv.Reset();
        ctx->swapChain = rebuiltSwapChain;
        ctx->rtv = rebuiltRTV;
        if (!AttachCompositionIfNeeded(ctx, ctx->swapChain.Get())) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to reattach transparent composition swapchain during resize");
            return;
        }
        ctx->width = w;
        ctx->height = h;
        return;
    }

    if (ctx->width == w && ctx->height == h && ctx->rtv) {
        return;
    }

    if (g_context) {
        g_context->OMSetRenderTargets(0, nullptr, nullptr);
        g_context->Flush();
    }
    ctx->rtv.Reset();

    HRESULT hr = ctx->swapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        ComPtr<IDXGISwapChain> rebuiltSwapChain;
        hr = CreateSwapChainForWindow(ctx->hwnd, w, h, &rebuiltSwapChain);
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to recreate swap chain after resize failure (0x%08X)", static_cast<unsigned int>(hr));
            return;
        }

        ComPtr<ID3D11Texture2D> backBuffer;
        hr = rebuiltSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to get back buffer after resize failure (0x%08X)", static_cast<unsigned int>(hr));
            return;
        }

        ComPtr<ID3D11RenderTargetView> rebuiltRTV;
        hr = g_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &rebuiltRTV);
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to create render target view after resize failure (0x%08X)", static_cast<unsigned int>(hr));
            return;
        }

        if (g_context) {
            g_context->OMSetRenderTargets(0, nullptr, nullptr);
            g_context->Flush();
        }
        ctx->rtv.Reset();
        ctx->swapChain = rebuiltSwapChain;
        ctx->rtv = rebuiltRTV;
        if (!AttachCompositionIfNeeded(ctx, ctx->swapChain.Get())) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to reattach transparent composition swapchain after resize failure");
            return;
        }
        ctx->width = w;
        ctx->height = h;
        return;
    }

    ComPtr<ID3D11Texture2D> backBuffer;
    hr = ctx->swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) {
        ComPtr<IDXGISwapChain> rebuiltSwapChain;
        hr = CreateSwapChainForWindow(ctx->hwnd, w, h, &rebuiltSwapChain);
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to recreate swap chain after back-buffer fetch failure (0x%08X)", static_cast<unsigned int>(hr));
            return;
        }

        ComPtr<ID3D11Texture2D> rebuiltBackBuffer;
        hr = rebuiltSwapChain->GetBuffer(0, IID_PPV_ARGS(&rebuiltBackBuffer));
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to get rebuilt back buffer (0x%08X)", static_cast<unsigned int>(hr));
            return;
        }

        ComPtr<ID3D11RenderTargetView> rebuiltRTV;
        hr = g_device->CreateRenderTargetView(rebuiltBackBuffer.Get(), nullptr, &rebuiltRTV);
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to create rebuilt render target view (0x%08X)", static_cast<unsigned int>(hr));
            return;
        }

        if (g_context) {
            g_context->OMSetRenderTargets(0, nullptr, nullptr);
            g_context->Flush();
        }
        ctx->rtv.Reset();
        ctx->swapChain = rebuiltSwapChain;
        ctx->rtv = rebuiltRTV;
        if (!AttachCompositionIfNeeded(ctx, ctx->swapChain.Get())) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to reattach transparent composition swapchain after back-buffer failure");
            return;
        }
        ctx->width = w;
        ctx->height = h;
        return;
    }

    hr = g_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &ctx->rtv);
    if (FAILED(hr)) {
        ComPtr<IDXGISwapChain> rebuiltSwapChain;
        hr = CreateSwapChainForWindow(ctx->hwnd, w, h, &rebuiltSwapChain);
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to recreate swap chain after render-target creation failure (0x%08X)", static_cast<unsigned int>(hr));
            return;
        }

        ComPtr<ID3D11Texture2D> rebuiltBackBuffer;
        hr = rebuiltSwapChain->GetBuffer(0, IID_PPV_ARGS(&rebuiltBackBuffer));
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to get rebuilt back buffer after render-target creation failure (0x%08X)", static_cast<unsigned int>(hr));
            return;
        }

        ComPtr<ID3D11RenderTargetView> rebuiltRTV;
        hr = g_device->CreateRenderTargetView(rebuiltBackBuffer.Get(), nullptr, &rebuiltRTV);
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to create rebuilt render target view after render-target creation failure (0x%08X)", static_cast<unsigned int>(hr));
            return;
        }

        if (g_context) {
            g_context->OMSetRenderTargets(0, nullptr, nullptr);
            g_context->Flush();
        }
        ctx->rtv.Reset();
        ctx->swapChain = rebuiltSwapChain;
        ctx->rtv = rebuiltRTV;
        if (!AttachCompositionIfNeeded(ctx, ctx->swapChain.Get())) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to reattach transparent composition swapchain after render-target failure");
            return;
        }
        ctx->width = w;
        ctx->height = h;
        return;
    }

    ctx->width = w;
    ctx->height = h;
}

bool PresentRendererWindowing(HWND hwnd)
{
    auto ctx = FindContext(hwnd);
    if (!ctx || !ctx->swapChain || !ctx->rtv) return false;

    HRESULT hr = ctx->swapChain->Present(1, 0);
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogWarn("Present failed with HRESULT 0x%08X; attempting to rebuild the swap chain", static_cast<unsigned int>(hr));
        ComPtr<IDXGISwapChain> rebuiltSwapChain;
        hr = CreateSwapChainForWindow(ctx->hwnd, ctx->width ? ctx->width : 1, ctx->height ? ctx->height : 1, &rebuiltSwapChain);
        if (FAILED(hr)) {
            return false;
        }

        ComPtr<ID3D11Texture2D> backBuffer;
        hr = rebuiltSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) {
            return false;
        }

        ComPtr<ID3D11RenderTargetView> rebuiltRTV;
        hr = g_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &rebuiltRTV);
        if (FAILED(hr)) {
            return false;
        }

        if (g_context) {
            g_context->OMSetRenderTargets(0, nullptr, nullptr);
            g_context->Flush();
        }
        ctx->rtv.Reset();
        ctx->swapChain = rebuiltSwapChain;
        ctx->rtv = rebuiltRTV;
        if (!AttachCompositionIfNeeded(ctx, ctx->swapChain.Get())) {
            ::NeoWPF::Diagnostics::LogWarn("Failed to reattach transparent composition swapchain after present failure");
            return false;
        }
        return false;
    }

    return true;
}

unsigned int GetRendererWidth(HWND hwnd)
{
    auto ctx = FindContext(hwnd);
    return ctx ? ctx->width : 0;
}

unsigned int GetRendererHeight(HWND hwnd)
{
    auto ctx = FindContext(hwnd);
    return ctx ? ctx->height : 0;
}

ID3D11RenderTargetView* GetRendererTargetView(HWND hwnd)
{
    auto ctx = FindContext(hwnd);
    return ctx && ctx->rtv ? ctx->rtv.Get() : nullptr;
}

bool IsRendererTransparent(HWND hwnd)
{
    const auto ctx = FindContext(hwnd);
    return ctx && ctx->transparent;
}

}
