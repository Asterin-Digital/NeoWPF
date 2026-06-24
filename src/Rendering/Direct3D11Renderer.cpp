#include <NeoWPF/Rendering/Renderer.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>
#include <stdexcept>

using namespace NeoWPF::Rendering;
using Microsoft::WRL::ComPtr;

namespace {
    HWND g_hwnd = nullptr;
    ComPtr<ID3D11Device> g_device;
    ComPtr<ID3D11DeviceContext> g_context;
    ComPtr<IDXGISwapChain> g_swapChain;
    ComPtr<ID3D11RenderTargetView> g_rtv;
    D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;
    HMODULE g_d3d11Dll = nullptr;
    using PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN = decltype(&D3D11CreateDeviceAndSwapChain);
    PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN g_D3D11CreateDeviceAndSwapChain = nullptr;
}

void NeoWPF::Rendering::Renderer_Initialize(HWND hwnd)
{
    if (!g_d3d11Dll) {
        g_d3d11Dll = LoadLibraryW(L"d3d11.dll");
        if (!g_d3d11Dll) {
            throw std::runtime_error("Failed to load d3d11.dll");
        }
        g_D3D11CreateDeviceAndSwapChain = reinterpret_cast<PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN>(GetProcAddress(g_d3d11Dll, "D3D11CreateDeviceAndSwapChain"));
        if (!g_D3D11CreateDeviceAndSwapChain) {
            throw std::runtime_error("Failed to locate D3D11CreateDeviceAndSwapChain");
        }
    }

    RECT rc;
    GetClientRect(hwnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    UINT createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
    createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT hr = g_D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createFlags, nullptr, 0, D3D11_SDK_VERSION, &sd, &g_swapChain, &g_device, &g_featureLevel, &g_context);
    if (FAILED(hr)) throw std::runtime_error("D3D11 device/swapchain creation failed");
    ComPtr<ID3D11Texture2D> backBuf;
    hr = g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuf));
    if (FAILED(hr)) throw std::runtime_error("Failed to get back buffer");
    hr = g_device->CreateRenderTargetView(backBuf.Get(), nullptr, &g_rtv);
    if (FAILED(hr)) throw std::runtime_error("Failed to create RTV");
    D3D11_VIEWPORT vp = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
    g_context->RSSetViewports(1, &vp);
    g_hwnd = hwnd;
}

void NeoWPF::Rendering::Renderer_Shutdown()
{
    g_rtv.Reset();
    g_swapChain.Reset();
    g_context.Reset();
    g_device.Reset();
}

void NeoWPF::Rendering::Renderer_Resize(unsigned int w, unsigned int h)
{
    if (!g_swapChain) return;
    g_context->OMSetRenderTargets(0, nullptr, nullptr);
    g_rtv.Reset();
    HRESULT hr = g_swapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) return;
    ComPtr<ID3D11Texture2D> backBuf;
    hr = g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuf));
    if (FAILED(hr)) return;
    hr = g_device->CreateRenderTargetView(backBuf.Get(), nullptr, &g_rtv);
    if (FAILED(hr)) return;
    D3D11_VIEWPORT vp = {0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h), 0.0f, 1.0f};
    g_context->RSSetViewports(1, &vp);
}

void NeoWPF::Rendering::Renderer_BeginFrame()
{
}

void NeoWPF::Rendering::Renderer_Execute(const CommandList& cmd)
{
    if (!g_context || !g_rtv) return;
    float clearColor[4] = {0.125f, 0.125f, 0.125f, 1.0f};
    g_context->ClearRenderTargetView(g_rtv.Get(), clearColor);
    for (const auto& r : cmd.rects) {
        float a = ((r.color >> 24) & 0xFF) / 255.0f;
        float red = ((r.color >> 16) & 0xFF) / 255.0f;
        float green = ((r.color >> 8) & 0xFF) / 255.0f;
        float blue = (r.color & 0xFF) / 255.0f;
        float color[4] = {red, green, blue, a};
        g_context->ClearRenderTargetView(g_rtv.Get(), color);
    }
    for (const auto& q : cmd.texturedQuads) {
        uint32_t id = q.textureId;
        float r = ((id >> 16) & 0xFF) / 255.0f;
        float g = ((id >> 8) & 0xFF) / 255.0f;
        float b = (id & 0xFF) / 255.0f;
        float color[4] = {r, g, b, 1.0f};
        g_context->ClearRenderTargetView(g_rtv.Get(), color);
    }
}

uint32_t NeoWPF::Rendering::Renderer_CreateTexture(unsigned int w, unsigned int h, const void* rgbaData)
{
    return 1;
}

void NeoWPF::Rendering::Renderer_DestroyTexture(uint32_t id)
{
}

void NeoWPF::Rendering::Renderer_Present()
{
    if (g_swapChain) g_swapChain->Present(1, 0);
}
