#include <NeoWPF/Rendering/D3D11/WRendering.h>
#include <NeoWPF/Rendering/Renderer.h>

namespace NeoWPF::Rendering::D3D11 {

bool WRendering::Initialize(void* nativeWindowHandle)
{
    NeoWPF::Rendering::Renderer_Initialize(nativeWindowHandle);
    return NeoWPF::Rendering::Renderer_IsInitialized();
}

void WRendering::Shutdown() noexcept
{
    NeoWPF::Rendering::Renderer_Shutdown();
}

bool WRendering::IsInitialized() const noexcept
{
    return NeoWPF::Rendering::Renderer_IsInitialized();
}

void WRendering::Resize(unsigned int width, unsigned int height) noexcept
{
    NeoWPF::Rendering::Renderer_Resize(width, height);
}

void WRendering::BeginFrame()
{
    NeoWPF::Rendering::Renderer_BeginFrame();
}

void WRendering::Execute(const NeoWPF::Rendering::CommandList& commands)
{
    NeoWPF::Rendering::Renderer_Execute(commands);
}

void WRendering::Present()
{
    NeoWPF::Rendering::Renderer_Present();
}

std::uint32_t WRendering::CreateTexture(unsigned int width, unsigned int height, const void* rgbaData)
{
    return NeoWPF::Rendering::Renderer_CreateTexture(width, height, rgbaData);
}

void WRendering::DestroyTexture(std::uint32_t id) noexcept
{
    NeoWPF::Rendering::Renderer_DestroyTexture(id);
}

void WRendering::ClearTextures() noexcept
{
    NeoWPF::Rendering::Renderer_ClearTextures();
}

}
