#include <NeoWPF/Rendering/D3D11/WRendering.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <Windows.h>

namespace NeoWPF::Rendering::D3D11 {

void Initialize(HWND hwnd);
void Shutdown();
void Resize(HWND hwnd, unsigned int width, unsigned int height);
void BeginFrame(HWND hwnd);
void Execute(HWND hwnd, const NeoWPF::Rendering::CommandList& commands);
void Present(HWND hwnd);
std::uint32_t CreateTexture(unsigned int width, unsigned int height, const void* rgbaData);
void DestroyTexture(std::uint32_t id);
void ClearTextures();
bool IsInitialized();

}

namespace NeoWPF::Rendering::D3D11 {

bool WRendering::Initialize(const NeoWPF::Rendering::RenderSurface& surface)
{
    NeoWPF::Rendering::D3D11::Initialize(static_cast<HWND>(surface.nativeWindowHandle));
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

void WRendering::Resize(const NeoWPF::Rendering::RenderSurface& surface,
                        unsigned int width, unsigned int height) noexcept
{
    NeoWPF::Rendering::D3D11::Resize(static_cast<HWND>(surface.nativeWindowHandle), width, height);
}

void WRendering::BeginFrame(const NeoWPF::Rendering::RenderSurface& surface)
{
    NeoWPF::Rendering::D3D11::BeginFrame(static_cast<HWND>(surface.nativeWindowHandle));
}

void WRendering::Execute(const NeoWPF::Rendering::RenderSurface& surface,
                         const NeoWPF::Rendering::CommandList& commands)
{
    NeoWPF::Rendering::D3D11::Execute(static_cast<HWND>(surface.nativeWindowHandle), commands);
}

void WRendering::Present(const NeoWPF::Rendering::RenderSurface& surface)
{
    NeoWPF::Rendering::D3D11::Present(static_cast<HWND>(surface.nativeWindowHandle));
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
