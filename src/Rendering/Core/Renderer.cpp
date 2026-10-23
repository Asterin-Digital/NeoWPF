#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace {

struct WindowFrameState {
    std::uint64_t lastCommandHash = 0;
    bool hasCommandHash = false;
    unsigned int lastWidth = 0;
    unsigned int lastHeight = 0;
    bool hasSize = false;
    bool framePending = false;
    bool frameSubmitted = false;
};

std::unordered_map<void*, WindowFrameState> s_windowStates;

WindowFrameState& GetWindowFrameState(void* nativeWindowHandle)
{
    if (!nativeWindowHandle) {
        static WindowFrameState nullState;
        return nullState;
    }
    return s_windowStates[nativeWindowHandle];
}

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

void NeoWPF::Rendering::Renderer_Initialize(const RenderSurface& surface)
{
    if (surface.nativeWindowHandle) {
        if (auto* rendering = CurrentRendering()) {
            rendering->Initialize(surface);
        }
    }
}

void NeoWPF::Rendering::Renderer_Shutdown()
{
    if (auto* rendering = CurrentRendering()) {
        rendering->Shutdown();
    }
    s_windowStates.clear();
}

void NeoWPF::Rendering::Renderer_Resize(const RenderSurface& surface, unsigned int w, unsigned int h)
{
    auto* nativeWindowHandle = surface.nativeWindowHandle;
    if (!nativeWindowHandle) {
        return;
    }

    auto& state = GetWindowFrameState(nativeWindowHandle);
    if (state.hasSize && w == state.lastWidth && h == state.lastHeight) {
        return;
    }
    state.hasCommandHash = false;
    state.lastWidth = w;
    state.lastHeight = h;
    state.hasSize = true;
    if (auto* rendering = CurrentRendering()) {
        rendering->Resize(surface, w, h);
    }
}

void NeoWPF::Rendering::Renderer_BeginFrame(const RenderSurface& surface)
{
    auto* nativeWindowHandle = surface.nativeWindowHandle;
    if (!nativeWindowHandle) {
        return;
    }

    auto& state = GetWindowFrameState(nativeWindowHandle);
    state.framePending = true;
    state.frameSubmitted = false;
}

void NeoWPF::Rendering::Renderer_Execute(const RenderSurface& surface, const CommandList& cmd)
{
    auto* nativeWindowHandle = surface.nativeWindowHandle;
    if (!nativeWindowHandle) {
        return;
    }

    auto& state = GetWindowFrameState(nativeWindowHandle);
    if (!state.framePending) {
        return;
    }
    const auto commandHash = HashCommandList(cmd);
    if (state.hasCommandHash && commandHash == state.lastCommandHash) {
        state.framePending = false;
        return;
    }
    if (auto* rendering = CurrentRendering()) {
        rendering->BeginFrame(surface);
        rendering->Execute(surface, cmd);
        state.lastCommandHash = commandHash;
        state.hasCommandHash = true;
        state.frameSubmitted = true;
    }
    state.framePending = false;
}

void NeoWPF::Rendering::Renderer_Present(const RenderSurface& surface)
{
    auto* nativeWindowHandle = surface.nativeWindowHandle;
    if (!nativeWindowHandle) {
        return;
    }

    auto& state = GetWindowFrameState(nativeWindowHandle);
    if (state.frameSubmitted) {
        if (auto* rendering = CurrentRendering()) {
            rendering->Present(surface);
        }
    }
    state.frameSubmitted = false;
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
