#include <NeoWPF/Diagnostics/FailureHooks.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
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
#include <mutex>

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
    std::mutex g_deviceMutex;

    HMODULE g_d3d11Dll = nullptr;
    using PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN = decltype(&D3D11CreateDeviceAndSwapChain);
    PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN g_D3D11CreateDeviceAndSwapChain = nullptr;

    const char* GetRectVertexShaderSource();
    const char* GetRectPixelShaderSource();
}

namespace NeoWPF::Rendering::D3D11 {

static void ThrowIfFailed(HRESULT hr, const char* message)
{
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("%s (0x%08X)", message, static_cast<unsigned int>(hr));
        throw std::runtime_error(message);
    }
}

static void LoadLibraryOrThrow(HMODULE& module, const wchar_t* libraryName, const char* errorMessage)
{
    if (module) return;
    module = LoadLibraryW(libraryName);
    if (!module) {
        ::NeoWPF::Diagnostics::LogError("%s", errorMessage);
        throw std::runtime_error(errorMessage);
    }
}

static void LoadProcOrThrow(HMODULE module, void** proc, const char* procName, const char* errorMessage)
{
    if (*proc) return;
    *proc = reinterpret_cast<void*>(GetProcAddress(module, procName));
    if (!*proc) {
        ::NeoWPF::Diagnostics::LogError("%s", errorMessage);
        throw std::runtime_error(errorMessage);
    }
}

static DXGI_SWAP_CHAIN_DESC CreateSwapChainDesc(HWND hwnd, unsigned int width, unsigned int height, DXGI_FORMAT format, DXGI_SWAP_EFFECT swapEffect)
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = format;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.SwapEffect = swapEffect;
    return sd;
}

void CreateDeviceAndSwapChain(HWND hwnd, unsigned int width, unsigned int height, IDXGISwapChain** swapChain)
{
    NEO_WPF_STARTUP_ACTION("D3D11.CreateDeviceAndSwapChain");
    std::lock_guard<std::mutex> deviceLock(g_deviceMutex);
    ::NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
        "[D3D11] CreateDeviceAndSwapChain: begin (hwnd=%p size=%ux%u)", hwnd, width, height);
    if (!g_d3d11Dll) {
        if (::NeoWPF::Diagnostics::GetFailureHookPoint() == ::NeoWPF::Diagnostics::FailureHookPoint::LoadD3D11Dll) {
            ::NeoWPF::Diagnostics::LogError("Injected failure: Failed to load d3d11.dll");
            throw std::runtime_error("Injected failure: Failed to load d3d11.dll");
        }
        {
            NEO_WPF_STARTUP_ACTION("D3D11.LoadLibrary");
            LoadLibraryOrThrow(g_d3d11Dll, L"d3d11.dll", "Failed to load d3d11.dll");
        }
        {
            NEO_WPF_STARTUP_ACTION("D3D11.ResolveProc");
            LoadProcOrThrow(g_d3d11Dll, reinterpret_cast<void**>(&g_D3D11CreateDeviceAndSwapChain), "D3D11CreateDeviceAndSwapChain", "Failed to locate D3D11CreateDeviceAndSwapChain");
        }
    }

    width = (width == 0) ? 1 : width;
    height = (height == 0) ? 1 : height;
    DXGI_SWAP_CHAIN_DESC sd = CreateSwapChainDesc(hwnd, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL);

    const UINT baseCreateFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    HRESULT hr = E_FAIL;
    const auto hookPoint = ::NeoWPF::Diagnostics::GetFailureHookPoint();
    const bool injectedCreateDeviceFailure = hookPoint == ::NeoWPF::Diagnostics::FailureHookPoint::CreateD3D11Device;
    const bool injectedCreateSwapChainFailure = hookPoint == ::NeoWPF::Diagnostics::FailureHookPoint::CreateD3D11SwapChain;

    const auto createDeviceAndSwapChain = [&](UINT createFlags, D3D_DRIVER_TYPE driverType) {
        if (injectedCreateDeviceFailure || injectedCreateSwapChainFailure) {
            ::NeoWPF::Diagnostics::LogWarn("Injected failure: CreateD3D11Device%s", injectedCreateSwapChainFailure ? "/SwapChain" : "");
            return E_FAIL;
        }
        return g_D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, createFlags, nullptr, 0, D3D11_SDK_VERSION, &sd, swapChain, &g_device, &g_featureLevel, &g_context);
    };

#if defined(_DEBUG)
    {
        NEO_WPF_STARTUP_ACTION("D3D11.CreateDevice.Hardware.debug");
        hr = createDeviceAndSwapChain(baseCreateFlags | D3D11_CREATE_DEVICE_DEBUG, D3D_DRIVER_TYPE_HARDWARE);
    }
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogWarn("Hardware device creation failed (0x%08X). retrying without debug flags", static_cast<unsigned int>(hr));
        {
            NEO_WPF_STARTUP_ACTION("D3D11.CreateDevice.Hardware.retry");
            hr = createDeviceAndSwapChain(baseCreateFlags, D3D_DRIVER_TYPE_HARDWARE);
        }
    }
#else
    {
        NEO_WPF_STARTUP_ACTION("D3D11.CreateDevice.Hardware");
        hr = createDeviceAndSwapChain(baseCreateFlags, D3D_DRIVER_TYPE_HARDWARE);
    }
#endif

    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogWarn("Hardware D3D11 device creation failed (0x%08X); falling back to WARP.", static_cast<unsigned int>(hr));
        {
            NEO_WPF_STARTUP_ACTION("D3D11.CreateDevice.WARP");
            hr = createDeviceAndSwapChain(baseCreateFlags, D3D_DRIVER_TYPE_WARP);
        }
    }
    ThrowIfFailed(hr, "D3D11 device/swapchain creation failed");
    ::NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
        "[D3D11] CreateDeviceAndSwapChain: success (device ready)");
}

HRESULT CreateSwapChainForWindow(HWND hwnd, unsigned int width, unsigned int height, IDXGISwapChain** swapChain)
{
    if (::NeoWPF::Diagnostics::GetFailureHookPoint() == ::NeoWPF::Diagnostics::FailureHookPoint::CreateD3D11SwapChain) {
        ::NeoWPF::Diagnostics::LogError("Injected failure: CreateD3D11SwapChain");
        return E_FAIL;
    }

    if (!g_device || !swapChain) return E_INVALIDARG;
    width = (width == 0) ? 1 : width;
    height = (height == 0) ? 1 : height;

    ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = g_device.As(&dxgiDevice);
    if (FAILED(hr)) return hr;

    ComPtr<IDXGIAdapter> adapter;
    hr = dxgiDevice->GetAdapter(&adapter);
    if (FAILED(hr)) return hr;

    ComPtr<IDXGIFactory> factory;
    hr = adapter->GetParent(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) return hr;

    DXGI_SWAP_CHAIN_DESC sd = CreateSwapChainDesc(hwnd, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_EFFECT_FLIP_DISCARD);
    return factory->CreateSwapChain(g_device.Get(), &sd, swapChain);
}

void EnsureCompilerLoaded()
{
    if (g_d3dcompiler && g_D3DCompile) return;
    LoadLibraryOrThrow(g_d3dcompiler, L"d3dcompiler_47.dll", "d3dcompiler_47.dll not found; shader compilation cannot proceed");
    LoadProcOrThrow(g_d3dcompiler, reinterpret_cast<void**>(&g_D3DCompile), "D3DCompile", "D3DCompile entrypoint not found in d3dcompiler_47.dll");
}

void CompileShaders()
{
    if (g_vs && g_ps && g_inputLayout && g_vsText && g_psText && g_inputLayoutText && g_samplerState) return;
    const bool injectedCompileFailure = ::NeoWPF::Diagnostics::GetFailureHookPoint() == ::NeoWPF::Diagnostics::FailureHookPoint::CompileShaders;
    EnsureCompilerLoaded();

    std::string vsSrc = D3D11::GetRectVertexShaderSource();
    std::string psSrc = D3D11::GetRectPixelShaderSource();
    if (vsSrc.empty() || psSrc.empty()) {
        ::NeoWPF::Diagnostics::LogError("Embedded shader source is missing");
        throw std::runtime_error("Embedded shader source is missing");
    }

    auto compileShader = [&](const std::string& source, const char* entryPoint, const char* profile, ComPtr<ID3DBlob>& outputBlob, const char* failureMessage) {
        ComPtr<ID3DBlob> errBlob;
        HRESULT hr = injectedCompileFailure ? E_FAIL : g_D3DCompile(source.c_str(), source.size(), nullptr, nullptr, nullptr, entryPoint, profile, 0, 0, &outputBlob, &errBlob);
        if (FAILED(hr) || !outputBlob) {
            if (errBlob) {
                ::NeoWPF::Diagnostics::LogError("%s", static_cast<const char*>(errBlob->GetBufferPointer()));
            }
            throw std::runtime_error(failureMessage);
        }
    };

    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> psBlob;

    compileShader(vsSrc, "VSMain", "vs_4_0", vsBlob, "Vertex shader compilation failed");
    compileShader(psSrc, "PSMain", "ps_4_0", psBlob, "Pixel shader compilation failed");

    ThrowIfFailed(g_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_vs), "Failed to create vertex shader");

    D3D11_INPUT_ELEMENT_DESC elems[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    ThrowIfFailed(g_device->CreateInputLayout(elems, _countof(elems), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_inputLayout), "Failed to create input layout");
    ThrowIfFailed(g_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_ps), "Failed to create pixel shader");

    {
        std::string textVsSrc = D3D11::GetTextVertexShaderSource();
        std::string textPsSrc = D3D11::GetTextPixelShaderSource();
        ComPtr<ID3DBlob> textVsBlob;
        ComPtr<ID3DBlob> textPsBlob;

        compileShader(textVsSrc, "VSMain", "vs_4_0", textVsBlob, "Text vertex shader compilation failed");
        ThrowIfFailed(g_device->CreateVertexShader(textVsBlob->GetBufferPointer(), textVsBlob->GetBufferSize(), nullptr, &g_vsText), "Failed to create text vertex shader");

        D3D11_INPUT_ELEMENT_DESC textElems[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };
        ThrowIfFailed(g_device->CreateInputLayout(textElems, _countof(textElems), textVsBlob->GetBufferPointer(), textVsBlob->GetBufferSize(), &g_inputLayoutText), "Failed to create text input layout");

        compileShader(textPsSrc, "PSMain", "ps_4_0", textPsBlob, "Text pixel shader compilation failed");
        ThrowIfFailed(g_device->CreatePixelShader(textPsBlob->GetBufferPointer(), textPsBlob->GetBufferSize(), nullptr, &g_psText), "Failed to create text pixel shader");
    }

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ThrowIfFailed(g_device->CreateSamplerState(&sampDesc, &g_samplerState), "Failed to create sampler state");
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
    ThrowIfFailed(hr, "Failed to create vertex buffer");

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = 16;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = g_device->CreateBuffer(&cbd, nullptr, &g_constantBuffer);
    ThrowIfFailed(hr, "Failed to create constant buffer");
}

void InitializeCommonResources()
{
    CompileShaders();
    EnsureBuffers();
}

}
