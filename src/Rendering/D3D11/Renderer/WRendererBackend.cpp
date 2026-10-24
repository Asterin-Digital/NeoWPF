#include <NeoWPF/Rendering/Renderer.h>
#include <d3d11.h>
#include <wrl.h>
#include <Windows.h>

using Microsoft::WRL::ComPtr;
using namespace NeoWPF::Rendering;

namespace NeoWPF::Rendering::D3D11 {
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

    void InitializeRendererWindowing(HWND hwnd);
    void ShutdownRendererWindowing();
    void DestroyRendererWindowing(HWND hwnd) noexcept;
    void ResizeRendererWindowing(HWND hwnd, unsigned int w, unsigned int h);
    bool PresentRendererWindowing(HWND hwnd);
    unsigned int GetRendererWidth(HWND hwnd);
    unsigned int GetRendererHeight(HWND hwnd);
    ID3D11RenderTargetView* GetRendererTargetView(HWND hwnd);
    void ExecuteRendererDraw(HWND hwnd, const CommandList& cmd);
    void CreateDevice();
    HRESULT CreateSwapChainForWindow(HWND hwnd, unsigned int width, unsigned int height, IDXGISwapChain** swapChain);
    void InitializeCommonResources();
    ComPtr<ID3D11ShaderResourceView> GetTextureShaderResourceView(uint32_t id);
    void ClearTextures();
    bool IsInitialized();
}

namespace NeoWPF::Rendering::D3D11 {

bool InitializeDevice()
{
    if (!g_device) {
        CreateDevice();
        InitializeCommonResources();
    }
    return IsInitialized();
}

bool CreateSurface(HWND hwnd)
{
    InitializeRendererWindowing(hwnd);
    return IsInitialized();
}

void DestroySurface(HWND hwnd) noexcept
{
    DestroyRendererWindowing(hwnd);
}

void ShutdownDevice()
{
    ShutdownRendererWindowing();
    ClearTextures();
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
}

bool IsInitialized()
{
    return static_cast<bool>(g_device);
}

void Resize(HWND hwnd, unsigned int w, unsigned int h)
{
    ResizeRendererWindowing(hwnd, w, h);
}

void BeginFrame(HWND)
{ // leave it
}

void Execute(HWND hwnd, const CommandList& cmd)
{
    ExecuteRendererDraw(hwnd, cmd);
}

void Present(HWND hwnd)
{
    PresentRendererWindowing(hwnd);
}

}
