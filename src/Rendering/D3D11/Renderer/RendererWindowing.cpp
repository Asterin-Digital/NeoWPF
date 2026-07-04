#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <d3d11.h>
#include <dxgi.h>
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
    void CreateDeviceAndSwapChain(HWND hwnd, unsigned int width, unsigned int height, IDXGISwapChain** swapChain);
    HRESULT CreateSwapChainForWindow(HWND hwnd, unsigned int width, unsigned int height, IDXGISwapChain** swapChain);
    void InitializeCommonResources();
}

namespace {

struct RendererWindowContext {
    HWND hwnd = nullptr;
    ComPtr<IDXGISwapChain> swapChain;
    ComPtr<ID3D11RenderTargetView> rtv;
    unsigned int width = 0;
    unsigned int height = 0;
};

thread_local HWND g_currentWindow = nullptr;
std::unordered_map<HWND, std::shared_ptr<RendererWindowContext>> g_windowContexts;
std::mutex g_windowContextsMutex;

std::shared_ptr<RendererWindowContext> FindContext(HWND hwnd)
{
    if (!hwnd) return nullptr;
    std::lock_guard<std::mutex> lock(g_windowContextsMutex);
    auto it = g_windowContexts.find(hwnd);
    return it == g_windowContexts.end() ? nullptr : it->second;
}

std::shared_ptr<RendererWindowContext> CurrentContext()
{
    return FindContext(g_currentWindow);
}

void SetCurrentWindow(HWND hwnd)
{
    g_currentWindow = hwnd;
}

}

namespace NeoWPF::Rendering::D3D11 {

void InitializeRendererWindowing(HWND hwnd)
{
    if (!hwnd) return;
    if (FindContext(hwnd)) {
        SetCurrentWindow(hwnd);
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

    ComPtr<IDXGISwapChain> swapChain;
    if (!g_device) {
        CreateDeviceAndSwapChain(hwnd, width, height, &swapChain);
    } else {
        HRESULT hr = CreateSwapChainForWindow(hwnd, width, height, &swapChain);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create swap chain for window");
        }
    }

    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to get back buffer");
    }

    auto ctx = std::make_shared<RendererWindowContext>();
    ctx->hwnd = hwnd;
    ctx->swapChain = swapChain;
    ctx->width = width;
    ctx->height = height;

    hr = g_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &ctx->rtv);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create render target view");
    }

    {
        std::lock_guard<std::mutex> lock(g_windowContextsMutex);
        g_windowContexts[hwnd] = ctx;
    }

    SetCurrentWindow(hwnd);
    InitializeCommonResources();
}

void ShutdownRendererWindowing()
{
    std::lock_guard<std::mutex> lock(g_windowContextsMutex);
    for (auto& pair : g_windowContexts) {
        pair.second->rtv.Reset();
        pair.second->swapChain.Reset();
    }
    g_windowContexts.clear();
    g_currentWindow = nullptr;
}

void ResizeRendererWindowing(unsigned int w, unsigned int h)
{
    if (!g_currentWindow) return;
    auto ctx = FindContext(g_currentWindow);
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
        ctx->width = w;
        ctx->height = h;
        return;
    }

    if (ctx->width == w && ctx->height == h && ctx->rtv) {
        return;
    }

    if (g_context) {
        g_context->OMSetRenderTargets(0, nullptr, nullptr);
    }
    ctx->rtv.Reset();

    HRESULT hr = ctx->swapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
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

        ctx->rtv.Reset();
        ctx->swapChain = rebuiltSwapChain;
        ctx->rtv = rebuiltRTV;
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

        ctx->rtv.Reset();
        ctx->swapChain = rebuiltSwapChain;
        ctx->rtv = rebuiltRTV;
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

        ctx->rtv.Reset();
        ctx->swapChain = rebuiltSwapChain;
        ctx->rtv = rebuiltRTV;
        ctx->width = w;
        ctx->height = h;
        return;
    }

    ctx->width = w;
    ctx->height = h;
}

bool PresentRendererWindowing()
{
    auto ctx = CurrentContext();
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

        ctx->rtv.Reset();
        ctx->swapChain = rebuiltSwapChain;
        ctx->rtv = rebuiltRTV;
        return false;
    }

    return true;
}

unsigned int GetCurrentRendererWidth()
{
    auto ctx = CurrentContext();
    return ctx ? ctx->width : 0;
}

unsigned int GetCurrentRendererHeight()
{
    auto ctx = CurrentContext();
    return ctx ? ctx->height : 0;
}

ID3D11RenderTargetView* GetCurrentRendererTargetView()
{
    auto ctx = CurrentContext();
    return ctx && ctx->rtv ? ctx->rtv.Get() : nullptr;
}

}
