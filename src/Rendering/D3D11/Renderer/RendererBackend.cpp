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
    void ResizeRendererWindowing(unsigned int w, unsigned int h);
    bool PresentRendererWindowing();
    unsigned int GetCurrentRendererWidth();
    unsigned int GetCurrentRendererHeight();
    ID3D11RenderTargetView* GetCurrentRendererTargetView();
    void ExecuteRendererDraw(const CommandList& cmd);
    void CreateDeviceAndSwapChain(HWND hwnd, unsigned int width, unsigned int height, IDXGISwapChain** swapChain);
    HRESULT CreateSwapChainForWindow(HWND hwnd, unsigned int width, unsigned int height, IDXGISwapChain** swapChain);
    void InitializeCommonResources();
    ComPtr<ID3D11ShaderResourceView> GetTextureShaderResourceView(uint32_t id);
}

namespace NeoWPF::Rendering::D3D11 {

void Initialize(HWND hwnd)
{
    InitializeRendererWindowing(hwnd);
}

void Shutdown()
{
    ShutdownRendererWindowing();
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

void Resize(unsigned int w, unsigned int h)
{
    ResizeRendererWindowing(w, h);
}

void BeginFrame()
{
}

void Execute(const CommandList& cmd)
{
    ExecuteRendererDraw(cmd);
}

void Present()
{
    PresentRendererWindowing();
}

}
