#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>
#include <stdexcept>
#include <d3dcompiler.h>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <Windows.h>

using namespace NeoWPF::Rendering;
using Microsoft::WRL::ComPtr;

namespace NeoWPF::Rendering {
namespace D3D11 {
    const char* GetRectVertexShaderSource();
    const char* GetRectPixelShaderSource();
}
}

namespace {
    HWND g_hwnd = nullptr;
    ComPtr<ID3D11Device> g_device;
    ComPtr<ID3D11DeviceContext> g_context;
    ComPtr<IDXGISwapChain> g_swapChain;
    ComPtr<ID3D11RenderTargetView> g_rtv;
    ComPtr<ID3D11VertexShader> g_vs;
    ComPtr<ID3D11PixelShader> g_ps;
    ComPtr<ID3D11InputLayout> g_inputLayout;
    ComPtr<ID3D11Buffer> g_vertexBuffer;
    ComPtr<ID3D11Buffer> g_constantBuffer;
    HMODULE g_d3dcompiler = nullptr;
    using PFN_D3DCOMPILE = HRESULT(WINAPI*)(LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**);
    PFN_D3DCOMPILE g_D3DCompile = nullptr;
    D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;
    HMODULE g_d3d11Dll = nullptr;
    using PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN = decltype(&D3D11CreateDeviceAndSwapChain);
    PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN g_D3D11CreateDeviceAndSwapChain = nullptr;
}

namespace NeoWPF::Rendering {
namespace D3D11 {

void Initialize(HWND hwnd)
{
    if (!g_d3d11Dll) {
        g_d3d11Dll = LoadLibraryW(L"d3d11.dll");
        if (!g_d3d11Dll) {
            ::NeoWPF::Diagnostics::LogError("Failed to load d3d11.dll");
            throw std::runtime_error("Failed to load d3d11.dll");
        }
        g_D3D11CreateDeviceAndSwapChain = reinterpret_cast<PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN>(GetProcAddress(g_d3d11Dll, "D3D11CreateDeviceAndSwapChain"));
        if (!g_D3D11CreateDeviceAndSwapChain) {
            ::NeoWPF::Diagnostics::LogError("Failed to locate D3D11CreateDeviceAndSwapChain");
            throw std::runtime_error("Failed to locate D3D11CreateDeviceAndSwapChain");
        }
    }

    RECT rc;
    GetClientRect(hwnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
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

    if (!g_d3dcompiler) {
        g_d3dcompiler = LoadLibraryW(L"d3dcompiler_47.dll");
        if (!g_d3dcompiler) {
            ::NeoWPF::Diagnostics::LogError("d3dcompiler_47.dll not found; shader compilation cannot proceed");
            throw std::runtime_error("d3dcompiler_47.dll not found");
        }
        g_D3DCompile = reinterpret_cast<PFN_D3DCOMPILE>(GetProcAddress(g_d3dcompiler, "D3DCompile"));
        if (!g_D3DCompile) {
            ::NeoWPF::Diagnostics::LogError("D3DCompile entrypoint not found in d3dcompiler_47.dll");
            throw std::runtime_error("D3DCompile entrypoint not found");
        }
    }

    std::string vsSrc = GetRectVertexShaderSource();
    std::string psSrc = GetRectPixelShaderSource();
    if (vsSrc.empty() || psSrc.empty()) {
        ::NeoWPF::Diagnostics::LogError("Embedded shader source is missing");
        throw std::runtime_error("Embedded shader source is missing");
    }

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errBlob = nullptr;

    {
        HRESULT hr = g_D3DCompile(vsSrc.c_str(), vsSrc.size(), nullptr, nullptr, nullptr, "VSMain", "vs_4_0", 0, 0, &vsBlob, &errBlob);
        if (FAILED(hr) || !vsBlob) {
            if (errBlob) {
                ::NeoWPF::Diagnostics::LogError("Vertex shader compile error: %s", static_cast<const char*>(errBlob->GetBufferPointer()));
                errBlob->Release();
            } else {
                ::NeoWPF::Diagnostics::LogError("Vertex shader compilation failed with HRESULT 0x%08X", static_cast<unsigned int>(hr));
            }
            throw std::runtime_error("Vertex shader compilation failed");
        }
    }
    {
        HRESULT hr = g_D3DCompile(psSrc.c_str(), psSrc.size(), nullptr, nullptr, nullptr, "PSMain", "ps_4_0", 0, 0, &psBlob, &errBlob);
        if (FAILED(hr) || !psBlob) {
            if (errBlob) {
                ::NeoWPF::Diagnostics::LogError("Pixel shader compile error: %s", static_cast<const char*>(errBlob->GetBufferPointer()));
                errBlob->Release();
            } else {
                ::NeoWPF::Diagnostics::LogError("Pixel shader compilation failed with HRESULT 0x%08X", static_cast<unsigned int>(hr));
            }
            if (vsBlob) vsBlob->Release();
            throw std::runtime_error("Pixel shader compilation failed");
        }
    }
    if (vsBlob) {
        g_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_vs);

        D3D11_INPUT_ELEMENT_DESC elems[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };
        g_device->CreateInputLayout(elems, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_inputLayout);
    }
    if (psBlob) {
        g_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_ps);
    }
    if (vsBlob) vsBlob->Release();
    if (psBlob) psBlob->Release();

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(float) * 6 * 6 * 1024;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    g_device->CreateBuffer(&bd, nullptr, &g_vertexBuffer);

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = 16;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    g_device->CreateBuffer(&cbd, nullptr, &g_constantBuffer);
}

void Shutdown()
{
    g_rtv.Reset();
    g_swapChain.Reset();
    g_context.Reset();
    g_device.Reset();
}

void Resize(unsigned int w, unsigned int h)
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

void BeginFrame()
{
}

void Execute(const CommandList& cmd)
{
    if (!g_context || !g_rtv) return;
    float clearColor[4] = {0.125f, 0.125f, 0.125f, 1.0f};
    g_context->ClearRenderTargetView(g_rtv.Get(), clearColor);

    struct Vertex { float x, y; float r, g, b, a; };
    std::vector<Vertex> verts;
    verts.reserve(cmd.rects.size() * 6 + cmd.texturedQuads.size() * 6);

    auto pushRect = [&](float x, float y, float w, float h, float r, float g, float b, float a) {
        verts.push_back({x, y, r, g, b, a});
        verts.push_back({x + w, y, r, g, b, a});
        verts.push_back({x + w, y + h, r, g, b, a});
        verts.push_back({x + w, y + h, r, g, b, a});
        verts.push_back({x, y + h, r, g, b, a});
        verts.push_back({x, y, r, g, b, a});
    };

    for (const auto& r : cmd.rects) {
        float a = ((r.color >> 24) & 0xFF) / 255.0f;
        float red = ((r.color >> 16) & 0xFF) / 255.0f;
        float green = ((r.color >> 8) & 0xFF) / 255.0f;
        float blue = (r.color & 0xFF) / 255.0f;
        pushRect(r.x, r.y, r.w, r.h, red, green, blue, a);
    }
    for (const auto& q : cmd.texturedQuads) {
        uint32_t id = q.textureId;
        float r = ((id >> 16) & 0xFF) / 255.0f;
        float g = ((id >> 8) & 0xFF) / 255.0f;
        float b = (id & 0xFF) / 255.0f;
        pushRect(q.x, q.y, q.w, q.h, r, g, b, 1.0f);
    }

    if (!verts.empty() && g_vs && g_ps && g_inputLayout && g_vertexBuffer) {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (SUCCEEDED(g_context->Map(g_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            float vp[2] = {0.0f, 0.0f};
            D3D11_VIEWPORT view;
            UINT num = 1;
            g_context->RSGetViewports(&num, &view);
            vp[0] = view.Width;
            vp[1] = view.Height;
            memcpy(mapped.pData, vp, sizeof(vp));
            g_context->Unmap(g_constantBuffer.Get(), 0);
        }

        D3D11_MAPPED_SUBRESOURCE vmap = {};
        if (SUCCEEDED(g_context->Map(g_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vmap))) {
            memcpy(vmap.pData, verts.data(), verts.size() * sizeof(Vertex));
            g_context->Unmap(g_vertexBuffer.Get(), 0);
        }

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        g_context->IASetInputLayout(g_inputLayout.Get());
        g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        g_context->IASetVertexBuffers(0, 1, g_vertexBuffer.GetAddressOf(), &stride, &offset);
        g_context->VSSetShader(g_vs.Get(), nullptr, 0);
        g_context->PSSetShader(g_ps.Get(), nullptr, 0);
        g_context->VSSetConstantBuffers(0, 1, g_constantBuffer.GetAddressOf());
        g_context->Draw(static_cast<UINT>(verts.size()), 0);
    }
}

uint32_t CreateTexture(unsigned int w, unsigned int h, const void* rgbaData)
{
    return 1;
}

void DestroyTexture(uint32_t id)
{
}

void Present()
{
    if (g_swapChain) g_swapChain->Present(1, 0);
}

} // namespace D3D11
} // namespace NeoWPF::Rendering
