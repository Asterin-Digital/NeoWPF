#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Diagnostics/Logger.h>

namespace {

NeoWPF::Contracts::IRendering* CurrentRendering() noexcept
{
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    return platform ? &platform->Rendering() : nullptr;
}

}

void NeoWPF::Rendering::Renderer_Initialize(void* nativeWindowHandle)
{
    if (auto* rendering = CurrentRendering()) {
        rendering->Initialize(nativeWindowHandle);
    }
}

void NeoWPF::Rendering::Renderer_Shutdown()
{
    if (auto* rendering = CurrentRendering()) {
        rendering->Shutdown();
    }
}

void NeoWPF::Rendering::Renderer_Resize(unsigned int w, unsigned int h)
{
    if (auto* rendering = CurrentRendering()) {
        rendering->Resize(w, h);
    }
}

void NeoWPF::Rendering::Renderer_BeginFrame()
{
    if (auto* rendering = CurrentRendering()) {
        rendering->BeginFrame();
    }
}

void NeoWPF::Rendering::Renderer_Execute(const CommandList& cmd)
{
    if (auto* rendering = CurrentRendering()) {
        rendering->Execute(cmd);
    }
}

void NeoWPF::Rendering::Renderer_Present()
{
    if (auto* rendering = CurrentRendering()) {
        rendering->Present();
    }
}

uint32_t NeoWPF::Rendering::Renderer_CreateTexture(unsigned int w, unsigned int h, const void* rgbaData)
{
    if (auto* rendering = CurrentRendering()) {
        const auto textureId = rendering->CreateTexture(w, h, rgbaData);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextRendering,
            NeoWPF::Diagnostics::Level::Debug,
            "[Renderer] CreateTexture size=%ux%u data=%p textureId=%u initialized=%d",
            w, h, rgbaData, textureId, rendering->IsInitialized() ? 1 : 0);
        return textureId;
    }
    NeoWPF::Diagnostics::LogError("[Renderer] CreateTexture no active rendering backend size=%ux%u", w, h);
    return 0;
}

void NeoWPF::Rendering::Renderer_DestroyTexture(uint32_t id)
{
    if (auto* rendering = CurrentRendering()) {
        rendering->DestroyTexture(id);
    }
}

void NeoWPF::Rendering::Renderer_ClearTextures()
{
    if (auto* rendering = CurrentRendering()) {
        rendering->ClearTextures();
    }
}

bool NeoWPF::Rendering::Renderer_IsInitialized()
{
    auto* rendering = CurrentRendering();
    return rendering && rendering->IsInitialized();
}
