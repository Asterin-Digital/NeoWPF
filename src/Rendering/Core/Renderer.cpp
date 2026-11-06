#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace {

struct WindowFrameState {
    std::uint64_t generationId = 0;
    std::uint64_t lastCommandHash = 0;
    bool hasCommandHash = false;
    unsigned int lastWidth = 0;
    unsigned int lastHeight = 0;
    bool hasSize = false;
    bool framePending = false;
    bool frameSubmitted = false;
    bool surfaceCreated = false;
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

WindowFrameState* FindWindowFrameState(const NeoWPF::Rendering::RenderSurface& surface) noexcept
{
    if (!surface.IsValid()) {
        return nullptr;
    }
    const auto state = s_windowStates.find(surface.nativeWindowHandle);
    if (state == s_windowStates.end() || state->second.generationId != surface.GetGenerationId()) {
        return nullptr;
    }
    return &state->second;
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

bool NeoWPF::Rendering::Renderer_Initialize(const RenderSurface& surface)
{
    auto surfaceUse = surface.TryAcquireUse();
    if (!surfaceUse.owns_lock()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Debug,
            "[Renderer/Surface] initialize skipped: handle=%p generation=%llu surface unavailable",
            surface.nativeWindowHandle,
            static_cast<unsigned long long>(surface.GetGenerationId()));
        return false;
    }
    const auto existingState = s_windowStates.find(surface.nativeWindowHandle);
    if (existingState != s_windowStates.end() &&
        existingState->second.generationId != surface.GetGenerationId()) {
        if (existingState->second.surfaceCreated) {
            return false;
        }
        s_windowStates.erase(existingState);
    }
    auto& state = GetWindowFrameState(surface.nativeWindowHandle);
    state.generationId = surface.GetGenerationId();
    if (state.surfaceCreated) {
        return true;
    }
    auto* rendering = CurrentRendering();
    if (!rendering) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Info,
            "[Renderer/Surface] initialize failed: handle=%p generation=%llu no rendering backend",
            surface.nativeWindowHandle,
            static_cast<unsigned long long>(surface.GetGenerationId()));
        return false;
    }
    if (!rendering->IsInitialized() && !rendering->InitializeDevice(surface)) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Info,
            "[Renderer/Surface] device initialization failed: handle=%p generation=%llu",
            surface.nativeWindowHandle,
            static_cast<unsigned long long>(surface.GetGenerationId()));
        return false;
    }
    state.surfaceCreated = rendering->CreateSurface(surface);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
        NeoWPF::Diagnostics::Level::Info,
        "[Renderer/Surface] create %s: handle=%p generation=%llu",
        state.surfaceCreated ? "complete" : "failed", surface.nativeWindowHandle,
        static_cast<unsigned long long>(surface.GetGenerationId()));
    return state.surfaceCreated;
}

void NeoWPF::Rendering::Renderer_DestroySurface(const RenderSurface& surface)
{
    if (!surface.nativeWindowHandle || !surface.generation) {
        return;
    }
    auto surfaceDestruction = surface.LockForDestruction();
    if (!surfaceDestruction.owns_lock()) {
        return;
    }
    const auto state = s_windowStates.find(surface.nativeWindowHandle);
    if (state == s_windowStates.end() || state->second.generationId != surface.GetGenerationId()) {
        return;
    }
    if (state->second.surfaceCreated) {
        if (auto* rendering = CurrentRendering()) {
            rendering->DestroySurface(surface);
        }
    }
    s_windowStates.erase(state);
}

void NeoWPF::Rendering::Renderer_Shutdown()
{
    if (auto* rendering = CurrentRendering()) {
        rendering->ShutdownDevice();
    }
    s_windowStates.clear();
}

void NeoWPF::Rendering::Renderer_Resize(const RenderSurface& surface, unsigned int w, unsigned int h)
{
    auto surfaceUse = surface.TryAcquireUse();
    if (!surfaceUse.owns_lock()) {
        return;
    }
    auto* state = FindWindowFrameState(surface);
    if (!state) {
        return;
    }

    if (state->hasSize && w == state->lastWidth && h == state->lastHeight) {
        return;
    }
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
        NeoWPF::Diagnostics::Level::Info,
        "[Renderer/Surface] resize: handle=%p generation=%llu extent=%ux%u",
        surface.nativeWindowHandle,
        static_cast<unsigned long long>(surface.GetGenerationId()), w, h);
    state->hasCommandHash = false;
    state->lastWidth = w;
    state->lastHeight = h;
    state->hasSize = true;
    if (auto* rendering = CurrentRendering()) {
        rendering->Resize(surface, w, h);
    }
}

void NeoWPF::Rendering::Renderer_BeginFrame(const RenderSurface& surface)
{
    auto surfaceUse = surface.TryAcquireUse();
    if (!surfaceUse.owns_lock()) {
        return;
    }
    auto* state = FindWindowFrameState(surface);
    if (!state) {
        return;
    }

    state->framePending = true;
    state->frameSubmitted = false;
}

void NeoWPF::Rendering::Renderer_Execute(const RenderSurface& surface, const CommandList& cmd)
{
    auto surfaceUse = surface.TryAcquireUse();
    if (!surfaceUse.owns_lock()) {
        return;
    }
    auto* state = FindWindowFrameState(surface);
    if (!state) {
        return;
    }

    if (!state->framePending) {
        return;
    }
    const auto commandHash = HashCommandList(cmd);
    if (state->hasCommandHash && commandHash == state->lastCommandHash) {
        state->framePending = false;
        return;
    }
    if (auto* rendering = CurrentRendering()) {
        rendering->BeginFrame(surface);
        rendering->Execute(surface, cmd);
        state->lastCommandHash = commandHash;
        state->hasCommandHash = true;
        state->frameSubmitted = true;
    }
    state->framePending = false;
}

void NeoWPF::Rendering::Renderer_Present(const RenderSurface& surface)
{
    auto surfaceUse = surface.TryAcquireUse();
    if (!surfaceUse.owns_lock()) {
        return;
    }
    auto* state = FindWindowFrameState(surface);
    if (!state) {
        return;
    }

    if (state->frameSubmitted) {
        if (auto* rendering = CurrentRendering()) {
            rendering->Present(surface);
        }
    }
    state->frameSubmitted = false;
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
