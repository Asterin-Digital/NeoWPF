#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <cstddef>
#include <cstdint>

namespace {

std::uint64_t s_lastCommandHash = 0;
bool s_hasCommandHash = false;
unsigned int s_lastWidth = 0;
unsigned int s_lastHeight = 0;
bool s_hasSize = false;
bool s_framePending = false;
bool s_frameSubmitted = false;

std::uint64_t HashCommandList(const NeoWPF::Rendering::CommandList& commands) noexcept
{
    constexpr std::uint64_t offset = 14695981039346656037ull;
    constexpr std::uint64_t prime = 1099511628211ull;
    std::uint64_t hash = offset;
    const auto* bytes = reinterpret_cast<const std::byte*>(commands.commands.data());
    const std::size_t byteCount = commands.commands.size() * sizeof(NeoWPF::Rendering::CommandList::Command);
    for (std::size_t index = 0; index < byteCount; ++index) {
        hash ^= static_cast<std::uint8_t>(bytes[index]);
        hash *= prime;
    }
    return hash;
}

NeoWPF::Contracts::IRendering* CurrentRendering() noexcept
{
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    return platform ? &platform->Rendering() : nullptr;
}

}

void NeoWPF::Rendering::Renderer_Initialize(void* nativeWindowHandle)
{
    s_hasCommandHash = false;
    s_hasSize = false;
    s_framePending = false;
    s_frameSubmitted = false;
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
    if (s_hasSize && w == s_lastWidth && h == s_lastHeight) {
        return;
    }
    s_hasCommandHash = false;
    s_lastWidth = w;
    s_lastHeight = h;
    s_hasSize = true;
    if (auto* rendering = CurrentRendering()) {
        rendering->Resize(w, h);
    }
}

void NeoWPF::Rendering::Renderer_BeginFrame()
{
    s_framePending = true;
    s_frameSubmitted = false;
}

void NeoWPF::Rendering::Renderer_Execute(const CommandList& cmd)
{
    if (!s_framePending) {
        return;
    }
    const auto commandHash = HashCommandList(cmd);
    if (s_hasCommandHash && commandHash == s_lastCommandHash) {
        s_framePending = false;
        return;
    }
    if (auto* rendering = CurrentRendering()) {
        rendering->BeginFrame();
        rendering->Execute(cmd);
        s_lastCommandHash = commandHash;
        s_hasCommandHash = true;
        s_frameSubmitted = true;
    }
    s_framePending = false;
}

void NeoWPF::Rendering::Renderer_Present()
{
    if (s_frameSubmitted) {
        if (auto* rendering = CurrentRendering()) {
            rendering->Present();
        }
    }
    s_frameSubmitted = false;
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
