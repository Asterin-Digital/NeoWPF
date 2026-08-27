#include <NeoWPF/Rendering/D3D11/WRendering.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <Windows.h>

namespace NeoWPF::Rendering::D3D11 {

void Initialize(HWND hwnd);
void Shutdown();
void Resize(unsigned int width, unsigned int height);
void BeginFrame();
void Execute(const NeoWPF::Rendering::CommandList& commands);
void Present();
std::uint32_t CreateTexture(unsigned int width, unsigned int height, const void* rgbaData);
void DestroyTexture(std::uint32_t id);
void ClearTextures();
bool IsInitialized();

}

namespace NeoWPF::Rendering::D3D11 {

bool WRendering::Initialize(void* nativeWindowHandle)
{
    NeoWPF::Rendering::D3D11::Initialize(static_cast<HWND>(nativeWindowHandle));
    return NeoWPF::Rendering::D3D11::IsInitialized();
}

void WRendering::Shutdown() noexcept
{
    NeoWPF::Rendering::D3D11::Shutdown();
}

bool WRendering::IsInitialized() const noexcept
{
    return NeoWPF::Rendering::D3D11::IsInitialized();
}

void WRendering::Resize(unsigned int width, unsigned int height) noexcept
{
    NeoWPF::Rendering::D3D11::Resize(width, height);
}

void WRendering::BeginFrame()
{
    NeoWPF::Rendering::D3D11::BeginFrame();
}

void WRendering::Execute(const NeoWPF::Rendering::CommandList& commands)
{
    NeoWPF::Rendering::D3D11::Execute(commands);
}

void WRendering::Present()
{
    NeoWPF::Rendering::D3D11::Present();
}

std::uint32_t WRendering::CreateTexture(unsigned int width, unsigned int height, const void* rgbaData)
{
    return NeoWPF::Rendering::D3D11::CreateTexture(width, height, rgbaData);
}

void WRendering::DestroyTexture(std::uint32_t id) noexcept
{
    NeoWPF::Rendering::D3D11::DestroyTexture(id);
}

void WRendering::ClearTextures() noexcept
{
    NeoWPF::Rendering::D3D11::ClearTextures();
}

}
