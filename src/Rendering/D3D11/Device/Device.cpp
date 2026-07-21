#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/ShaderSources.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>
#include <stdexcept>
#include <d3dcompiler.h>
#include <string>
#include <cstdint>
#include <Windows.h>

using Microsoft::WRL::ComPtr;
using namespace NeoWPF::Rendering;

namespace NeoWPF::Rendering::D3D11 {
    ComPtr<ID3D11Device> g_device;
    ComPtr<ID3D11DeviceContext> g_context;
    ComPtr<ID3D11VertexShader> g_vs;
    ComPtr<ID3D11PixelShader> g_ps;
    ComPtr<ID3D11InputLayout> g_inputLayout;
    ComPtr<ID3D11VertexShader> g_vsText;
    ComPtr<ID3D11PixelShader> g_psText;
    ComPtr<ID3D11InputLayout> g_inputLayoutText;
    ComPtr<ID3D11SamplerState> g_samplerState;
    ComPtr<ID3D11Buffer> g_vertexBuffer;
    ComPtr<ID3D11Buffer> g_constantBuffer;

    HMODULE g_d3dcompiler = nullptr;
    using PFN_D3DCOMPILE = HRESULT(WINAPI*)(LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**);
    PFN_D3DCOMPILE g_D3DCompile = nullptr;
    D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;

    HMODULE g_d3d11Dll = nullptr;
    using PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN = decltype(&D3D11CreateDeviceAndSwapChain);
    PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN g_D3D11CreateDeviceAndSwapChain = nullptr;

    const char* GetRectVertexShaderSource();
    const char* GetRectPixelShaderSource();
}

namespace NeoWPF::Rendering::D3D11 {

void CreateDeviceAndSwapChain(HWND hwnd, unsigned int width, unsigned int height, IDXGISwapChain** swapChain)
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

    const UINT baseCreateFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    HRESULT hr = E_FAIL;

#if defined(_DEBUG)
    const UINT debugCreateFlags = baseCreateFlags | D3D11_CREATE_DEVICE_DEBUG;
    hr = g_D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, debugCreateFlags, nullptr, 0, D3D11_SDK_VERSION, &sd, swapChain, &g_device, &g_featureLevel, &g_context);
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogWarn("Hardware device creation failed (0x%08X). retrying without debug flags", static_cast<unsigned int>(hr));
        hr = g_D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, baseCreateFlags, nullptr, 0, D3D11_SDK_VERSION, &sd, swapChain, &g_device, &g_featureLevel, &g_context);
    }
#else
    hr = g_D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, baseCreateFlags, nullptr, 0, D3D11_SDK_VERSION, &sd, swapChain, &g_device, &g_featureLevel, &g_context);
#endif

    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogWarn("Hardware D3D11 device creation failed (0x%08X); falling back to WARP.", static_cast<unsigned int>(hr));
        hr = g_D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, baseCreateFlags, nullptr, 0, D3D11_SDK_VERSION, &sd, swapChain, &g_device, &g_featureLevel, &g_context);
    }
    if (FAILED(hr)) {
        throw std::runtime_error("D3D11 device/swapchain creation failed");
    }
}

HRESULT CreateSwapChainForWindow(HWND hwnd, unsigned int width, unsigned int height, IDXGISwapChain** swapChain)
{
    if (!g_device || !swapChain) return E_FAIL;
    if (width == 0) width = 1;
    if (height == 0) height = 1;

    ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = g_device.As(&dxgiDevice);
    if (FAILED(hr)) return hr;

    ComPtr<IDXGIAdapter> adapter;
    hr = dxgiDevice->GetAdapter(&adapter);
    if (FAILED(hr)) return hr;

    ComPtr<IDXGIFactory> factory;
    hr = adapter->GetParent(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) return hr;

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
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    return factory->CreateSwapChain(g_device.Get(), &sd, swapChain);
}

void EnsureCompilerLoaded()
{
    if (g_d3dcompiler) return;
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

void CompileShaders()
{
    if (g_vs && g_ps && g_inputLayout && g_vsText && g_psText && g_inputLayoutText && g_samplerState) return;
    EnsureCompilerLoaded();

    std::string vsSrc = D3D11::GetRectVertexShaderSource();
    std::string psSrc = D3D11::GetRectPixelShaderSource();
    if (vsSrc.empty() || psSrc.empty()) {
        ::NeoWPF::Diagnostics::LogError("Embedded shader source is missing");
        throw std::runtime_error("Embedded shader source is missing");
    }

    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> psBlob;
    ComPtr<ID3DBlob> errBlob;

    HRESULT hr = g_D3DCompile(vsSrc.c_str(), vsSrc.size(), nullptr, nullptr, nullptr, "VSMain", "vs_4_0", 0, 0, &vsBlob, &errBlob);
    if (FAILED(hr) || !vsBlob) {
        if (errBlob) {
            ::NeoWPF::Diagnostics::LogError("Vertex shader compile error: %s", static_cast<const char*>(errBlob->GetBufferPointer()));
        }
        throw std::runtime_error("Vertex shader compilation failed");
    }

    hr = g_D3DCompile(psSrc.c_str(), psSrc.size(), nullptr, nullptr, nullptr, "PSMain", "ps_4_0", 0, 0, &psBlob, &errBlob);
    if (FAILED(hr) || !psBlob) {
        if (errBlob) {
            ::NeoWPF::Diagnostics::LogError("Pixel shader compile error: %s", static_cast<const char*>(errBlob->GetBufferPointer()));
        }
        throw std::runtime_error("Pixel shader compilation failed");
    }

    hr = g_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_vs);
    if (FAILED(hr)) throw std::runtime_error("Failed to create vertex shader");

    D3D11_INPUT_ELEMENT_DESC elems[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    hr = g_device->CreateInputLayout(elems, _countof(elems), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_inputLayout);
    if (FAILED(hr)) throw std::runtime_error("Failed to create input layout");

    hr = g_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_ps);
    if (FAILED(hr)) throw std::runtime_error("Failed to create pixel shader");

    {
        std::string textVsSrc = D3D11::GetTextVertexShaderSource();
        std::string textPsSrc = D3D11::GetTextPixelShaderSource();
        ComPtr<ID3DBlob> textVsBlob;
        ComPtr<ID3DBlob> textPsBlob;
        ComPtr<ID3DBlob> errBlob;

        hr = g_D3DCompile(textVsSrc.c_str(), textVsSrc.size(), nullptr, nullptr, nullptr, "VSMain", "vs_4_0", 0, 0, &textVsBlob, &errBlob);
        if (FAILED(hr) || !textVsBlob) {
            if (errBlob) {
                ::NeoWPF::Diagnostics::LogError("Text vertex shader compile error: %s", static_cast<const char*>(errBlob->GetBufferPointer()));
            }
            throw std::runtime_error("Text vertex shader compilation failed");
        }

        hr = g_device->CreateVertexShader(textVsBlob->GetBufferPointer(), textVsBlob->GetBufferSize(), nullptr, &g_vsText);
        if (FAILED(hr)) throw std::runtime_error("Failed to create text vertex shader");

        D3D11_INPUT_ELEMENT_DESC textElems[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };
        hr = g_device->CreateInputLayout(textElems, _countof(textElems), textVsBlob->GetBufferPointer(), textVsBlob->GetBufferSize(), &g_inputLayoutText);
        if (FAILED(hr)) throw std::runtime_error("Failed to create text input layout");

        hr = g_D3DCompile(textPsSrc.c_str(), textPsSrc.size(), nullptr, nullptr, nullptr, "PSMain", "ps_4_0", 0, 0, &textPsBlob, &errBlob);
        if (FAILED(hr) || !textPsBlob) {
            if (errBlob) {
                ::NeoWPF::Diagnostics::LogError("Text pixel shader compile error: %s", static_cast<const char*>(errBlob->GetBufferPointer()));
            }
            throw std::runtime_error("Text pixel shader compilation failed");
        }

        hr = g_device->CreatePixelShader(textPsBlob->GetBufferPointer(), textPsBlob->GetBufferSize(), nullptr, &g_psText);
        if (FAILED(hr)) throw std::runtime_error("Failed to create text pixel shader");
    }

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = g_device->CreateSamplerState(&sampDesc, &g_samplerState);
    if (FAILED(hr)) throw std::runtime_error("Failed to create sampler state");
}

void EnsureBuffers()
{
    if (g_vertexBuffer && g_constantBuffer) return;

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = 1 << 20; // 1 MiB for the shared UI vertex buffer
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    HRESULT hr = g_device->CreateBuffer(&bd, nullptr, &g_vertexBuffer);
    if (FAILED(hr)) throw std::runtime_error("Failed to create vertex buffer");

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = 16;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = g_device->CreateBuffer(&cbd, nullptr, &g_constantBuffer);
    if (FAILED(hr)) throw std::runtime_error("Failed to create constant buffer");
}

void InitializeCommonResources()
{
    CompileShaders();
    EnsureBuffers();
}

}
