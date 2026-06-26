#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>
#include <stdexcept>
#include <d3dcompiler.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <Windows.h>

using Microsoft::WRL::ComPtr;
using namespace NeoWPF::Rendering;

namespace NeoWPF::Rendering {
namespace D3D11 {
    const char* GetRectVertexShaderSource();
    const char* GetRectPixelShaderSource();

    extern ComPtr<ID3D11Device> g_device;
    extern ComPtr<ID3D11DeviceContext> g_context;
    extern ComPtr<ID3D11VertexShader> g_vs;
    extern ComPtr<ID3D11PixelShader> g_ps;
    extern ComPtr<ID3D11InputLayout> g_inputLayout;
    extern ComPtr<ID3D11VertexShader> g_vsText;
    extern ComPtr<ID3D11PixelShader> g_psText;
    extern ComPtr<ID3D11InputLayout> g_inputLayoutText;
    extern ComPtr<ID3D11SamplerState> g_samplerState;
    extern ComPtr<ID3D11Buffer> g_vertexBuffer;
    extern ComPtr<ID3D11Buffer> g_constantBuffer;
    extern HMODULE g_d3dcompiler;
    using PFN_D3DCOMPILE = HRESULT(WINAPI*)(LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**);
    extern PFN_D3DCOMPILE g_D3DCompile;
    extern D3D_FEATURE_LEVEL g_featureLevel;
    extern HMODULE g_d3d11Dll;
    using PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN = decltype(&D3D11CreateDeviceAndSwapChain);
    extern PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN g_D3D11CreateDeviceAndSwapChain;

    HRESULT CreateSwapChainForWindow(HWND hwnd, unsigned int width, unsigned int height, IDXGISwapChain** swapChain);
    void CreateDeviceAndSwapChain(HWND hwnd, unsigned int width, unsigned int height, IDXGISwapChain** swapChain);
    void EnsureCompilerLoaded();
    void CompileShaders();
    void EnsureBuffers();
    void InitializeCommonResources();
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTextureShaderResourceView(uint32_t id);
}
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

using namespace NeoWPF::Rendering::D3D11;

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

void CreateWindowContext(HWND hwnd)
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
        if (FAILED(hr)) throw std::runtime_error("Failed to create swap chain for window");
    }

    ComPtr<ID3D11Texture2D> backBuf;
    HRESULT hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuf));
    if (FAILED(hr)) throw std::runtime_error("Failed to get back buffer");

    auto ctx = std::make_shared<RendererWindowContext>();
    ctx->hwnd = hwnd;
    ctx->swapChain = swapChain;
    ctx->width = width;
    ctx->height = height;

    hr = g_device->CreateRenderTargetView(backBuf.Get(), nullptr, &ctx->rtv);
    if (FAILED(hr)) throw std::runtime_error("Failed to create render target view");

    {
        std::lock_guard<std::mutex> lock(g_windowContextsMutex);
        g_windowContexts[hwnd] = ctx;
    }

    SetCurrentWindow(hwnd);
    InitializeCommonResources();
}

void ResizeWindowContext(HWND hwnd, unsigned int w, unsigned int h)
{
    if (w == 0 || h == 0) return;
    auto ctx = FindContext(hwnd);
    if (!ctx || !ctx->swapChain) return;

    g_context->OMSetRenderTargets(0, nullptr, nullptr);
    ctx->rtv.Reset();

    HRESULT hr = ctx->swapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) return;

    ComPtr<ID3D11Texture2D> backBuf;
    hr = ctx->swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuf));
    if (FAILED(hr)) return;

    hr = g_device->CreateRenderTargetView(backBuf.Get(), nullptr, &ctx->rtv);
    if (FAILED(hr)) return;

    ctx->width = w;
    ctx->height = h;
}

void DestroyWindowContext(HWND hwnd)
{
    std::lock_guard<std::mutex> lock(g_windowContextsMutex);
    auto it = g_windowContexts.find(hwnd);
    if (it == g_windowContexts.end()) return;

    it->second->rtv.Reset();
    it->second->swapChain.Reset();
    g_windowContexts.erase(it);
    if (g_currentWindow == hwnd) {
        g_currentWindow = nullptr;
        if (!g_windowContexts.empty()) {
            g_currentWindow = g_windowContexts.begin()->first;
        }
    }
}

} // namespace

namespace NeoWPF::Rendering {
namespace D3D11 {

void Initialize(HWND hwnd)
{
    CreateWindowContext(hwnd);
}

void Shutdown()
{
    std::lock_guard<std::mutex> lock(g_windowContextsMutex);
    for (auto& pair : g_windowContexts) {
        pair.second->rtv.Reset();
        pair.second->swapChain.Reset();
    }
    g_windowContexts.clear();
    g_context.Reset();
    g_device.Reset();
    g_vs.Reset();
    g_ps.Reset();
    g_vsText.Reset();
    g_psText.Reset();
    g_inputLayout.Reset();
    g_inputLayoutText.Reset();
    g_samplerState.Reset();
    g_vertexBuffer.Reset();
    g_constantBuffer.Reset();
    g_currentWindow = nullptr;
}

void Resize(unsigned int w, unsigned int h)
{
    if (!g_currentWindow) return;
    ResizeWindowContext(g_currentWindow, w, h);
}

void BeginFrame()
{
}

void Execute(const CommandList& cmd)
{
    auto ctx = CurrentContext();
    if (!ctx || !ctx->rtv) return;

    g_context->OMSetRenderTargets(1, ctx->rtv.GetAddressOf(), nullptr);
    D3D11_VIEWPORT vp = {0.0f, 0.0f, static_cast<float>(ctx->width), static_cast<float>(ctx->height), 0.0f, 1.0f};
    g_context->RSSetViewports(1, &vp);

    float clearColor[4] = {0.125f, 0.125f, 0.125f, 1.0f};
    g_context->ClearRenderTargetView(ctx->rtv.Get(), clearColor);

    struct Vertex { float x, y; float r, g, b, a; };
    std::vector<Vertex> rectVerts;
    rectVerts.reserve(cmd.rects.size() * 6);

    auto pushRect = [&](float x, float y, float w, float h, float r, float g, float b, float a) {
        rectVerts.push_back({x, y, r, g, b, a});
        rectVerts.push_back({x + w, y, r, g, b, a});
        rectVerts.push_back({x + w, y + h, r, g, b, a});
        rectVerts.push_back({x + w, y + h, r, g, b, a});
        rectVerts.push_back({x, y + h, r, g, b, a});
        rectVerts.push_back({x, y, r, g, b, a});
    };

    for (const auto& r : cmd.rects) {
        float a = ((r.color >> 24) & 0xFF) / 255.0f;
        float red = ((r.color >> 16) & 0xFF) / 255.0f;
        float green = ((r.color >> 8) & 0xFF) / 255.0f;
        float blue = (r.color & 0xFF) / 255.0f;
        pushRect(r.x, r.y, r.w, r.h, red, green, blue, a);
    }

    if (!rectVerts.empty() && g_vs && g_ps && g_inputLayout && g_vertexBuffer) {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (SUCCEEDED(g_context->Map(g_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            float vp[2] = {static_cast<float>(ctx->width), static_cast<float>(ctx->height)};
            memcpy(mapped.pData, vp, sizeof(vp));
            g_context->Unmap(g_constantBuffer.Get(), 0);
        }

        D3D11_MAPPED_SUBRESOURCE vmap = {};
        if (SUCCEEDED(g_context->Map(g_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vmap))) {
            memcpy(vmap.pData, rectVerts.data(), rectVerts.size() * sizeof(Vertex));
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
        g_context->Draw(static_cast<UINT>(rectVerts.size()), 0);
    }

    struct TextVertex { float x, y; float u, v; float r, g, b, a; };
    std::vector<TextVertex> textVerts;
    textVerts.reserve(cmd.texturedQuads.size() * 6);

    auto pushTextQuad = [&](float x, float y, float w, float h, float u0, float v0, float u1, float v1) {
        textVerts.push_back({x, y, u0, v0, 1.0f, 1.0f, 1.0f, 1.0f});
        textVerts.push_back({x + w, y, u1, v0, 1.0f, 1.0f, 1.0f, 1.0f});
        textVerts.push_back({x + w, y + h, u1, v1, 1.0f, 1.0f, 1.0f, 1.0f});
        textVerts.push_back({x + w, y + h, u1, v1, 1.0f, 1.0f, 1.0f, 1.0f});
        textVerts.push_back({x, y + h, u0, v1, 1.0f, 1.0f, 1.0f, 1.0f});
        textVerts.push_back({x, y, u0, v0, 1.0f, 1.0f, 1.0f, 1.0f});
    };

    for (const auto& q : cmd.texturedQuads) {
        pushTextQuad(q.x, q.y, q.w, q.h, q.u0, q.v0, q.u1, q.v1);
    }

    if (!textVerts.empty() && g_vsText && g_psText && g_inputLayoutText && g_vertexBuffer && g_samplerState) {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (SUCCEEDED(g_context->Map(g_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            float vp[2] = {static_cast<float>(ctx->width), static_cast<float>(ctx->height)};
            memcpy(mapped.pData, vp, sizeof(vp));
            g_context->Unmap(g_constantBuffer.Get(), 0);
        }

        D3D11_MAPPED_SUBRESOURCE vmap = {};
        if (SUCCEEDED(g_context->Map(g_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vmap))) {
            memcpy(vmap.pData, textVerts.data(), textVerts.size() * sizeof(TextVertex));
            g_context->Unmap(g_vertexBuffer.Get(), 0);
        }

        UINT stride = sizeof(TextVertex);
        UINT offset = 0;
        g_context->IASetInputLayout(g_inputLayoutText.Get());
        g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        g_context->IASetVertexBuffers(0, 1, g_vertexBuffer.GetAddressOf(), &stride, &offset);
        g_context->VSSetShader(g_vsText.Get(), nullptr, 0);
        g_context->PSSetShader(g_psText.Get(), nullptr, 0);
        g_context->VSSetConstantBuffers(0, 1, g_constantBuffer.GetAddressOf());
        g_context->PSSetSamplers(0, 1, g_samplerState.GetAddressOf());

        UINT vertexStart = 0;
        for (size_t qi = 0; qi < cmd.texturedQuads.size();) {
            uint32_t texId = cmd.texturedQuads[qi].textureId;
            size_t batchCount = 1;
            while (qi + batchCount < cmd.texturedQuads.size() && cmd.texturedQuads[qi + batchCount].textureId == texId) {
                batchCount++;
            }

            ComPtr<ID3D11ShaderResourceView> srv = D3D11::GetTextureShaderResourceView(texId);
            if (srv) {
                g_context->PSSetShaderResources(0, 1, srv.GetAddressOf());
                g_context->Draw(static_cast<UINT>(batchCount * 6), vertexStart);
            }

            vertexStart += static_cast<UINT>(batchCount * 6);
            qi += batchCount;
        }
    }
}

void Present()
{
    auto ctx = CurrentContext();
    if (!ctx || !ctx->swapChain) return;
    ctx->swapChain->Present(1, 0);
}

} // namespace D3D11
} // namespace NeoWPF::Rendering
