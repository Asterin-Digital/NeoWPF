#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <Windows.h>

namespace NeoWPF::Rendering {
namespace D3D11 {
    void Initialize(HWND hwnd);
    void Shutdown();
    void Resize(unsigned int w, unsigned int h);
    void BeginFrame();
    void Execute(const CommandList& cmd);
    void Present();
    uint32_t CreateTexture(unsigned int w, unsigned int h, const void* rgbaData);
    void DestroyTexture(uint32_t id);
}
}

void NeoWPF::Rendering::Renderer_Initialize(HWND hwnd)
{
    D3D11::Initialize(hwnd);
}

void NeoWPF::Rendering::Renderer_Shutdown()
{
    D3D11::Shutdown();
}

void NeoWPF::Rendering::Renderer_Resize(unsigned int w, unsigned int h)
{
    D3D11::Resize(w, h);
}

void NeoWPF::Rendering::Renderer_BeginFrame()
{
    D3D11::BeginFrame();
}

void NeoWPF::Rendering::Renderer_Execute(const CommandList& cmd)
{
    D3D11::Execute(cmd);
}

void NeoWPF::Rendering::Renderer_Present()
{
    D3D11::Present();
}

uint32_t NeoWPF::Rendering::Renderer_CreateTexture(unsigned int w, unsigned int h, const void* rgbaData)
{
    return D3D11::CreateTexture(w, h, rgbaData);
}

void NeoWPF::Rendering::Renderer_DestroyTexture(uint32_t id)
{
    D3D11::DestroyTexture(id);
}
