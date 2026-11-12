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

void HashBytes(std::uint64_t& hash, const void* value, std::size_t size) noexcept
{
    constexpr std::uint64_t prime = 1099511628211ull;
    const auto* bytes = static_cast<const std::byte*>(value);
    for (std::size_t index = 0; index < size; ++index) {
        hash ^= static_cast<std::uint8_t>(bytes[index]);
        hash *= prime;
    }
}

template <typename T>
void HashValue(std::uint64_t& hash, const T& value) noexcept
{
    HashBytes(hash, &value, sizeof(value));
}

void HashValue(std::uint64_t& hash, bool value) noexcept
{
    const std::uint8_t normalized = value ? 1u : 0u;
    HashValue(hash, normalized);
}

void HashClip(std::uint64_t& hash, const NeoWPF::Rendering::ClipRect& clip) noexcept
{
    HashValue(hash, clip.x);
    HashValue(hash, clip.y);
    HashValue(hash, clip.w);
    HashValue(hash, clip.h);
    HashValue(hash, clip.active);
}

std::uint64_t HashCommandList(const NeoWPF::Rendering::CommandList& commands) noexcept
{
    using CommandList = NeoWPF::Rendering::CommandList;
    constexpr std::uint64_t offset = 14695981039346656037ull;
    std::uint64_t hash = offset;
    HashValue(hash, commands.commands.size());

    for (const auto& command : commands.commands) {
        HashValue(hash, command.kind);
        switch (command.kind) {
        case CommandList::CommandKind::Rect: {
            const auto& rect = command.rect;
            HashValue(hash, rect.x);
            HashValue(hash, rect.y);
            HashValue(hash, rect.w);
            HashValue(hash, rect.h);
            HashValue(hash, rect.paint.kind);
            HashValue(hash, rect.paint.kind);
            HashValue(hash, rect.paint.color);
            HashValue(hash, rect.opacity);
            HashClip(hash, rect.clip);
            HashValue(hash, rect.rotationRadians);
            HashValue(hash, rect.pivotX);
            HashValue(hash, rect.pivotY);
            break;
        }
        case CommandList::CommandKind::RoundedRect: {
            const auto& rect = command.roundedRect;
            HashValue(hash, rect.x);
            HashValue(hash, rect.y);
            HashValue(hash, rect.w);
            HashValue(hash, rect.h);
            HashValue(hash, rect.topLeft);
            HashValue(hash, rect.topRight);
            HashValue(hash, rect.bottomRight);
            HashValue(hash, rect.bottomLeft);
            HashValue(hash, rect.paint.kind);
            HashValue(hash, rect.paint.color);
            HashValue(hash, rect.opacity);
            HashClip(hash, rect.clip);
            HashValue(hash, rect.rotationRadians);
            HashValue(hash, rect.pivotX);
            HashValue(hash, rect.pivotY);
            break;
        }
        case CommandList::CommandKind::Arc: {
            const auto& arc = command.arc;
            HashValue(hash, arc.centerX);
            HashValue(hash, arc.centerY);
            HashValue(hash, arc.radius);
            HashValue(hash, arc.thickness);
            HashValue(hash, arc.startAngleRadians);
            HashValue(hash, arc.sweepAngleRadians);
            HashValue(hash, arc.paint.kind);
            HashValue(hash, arc.paint.color);
            HashValue(hash, arc.opacity);
            HashClip(hash, arc.clip);
            break;
        }
        case CommandList::CommandKind::TexturedQuad: {
            const auto& quad = command.texturedQuad;
            HashValue(hash, quad.x);
            HashValue(hash, quad.y);
            HashValue(hash, quad.w);
            HashValue(hash, quad.h);
            HashValue(hash, quad.u0);
            HashValue(hash, quad.v0);
            HashValue(hash, quad.u1);
            HashValue(hash, quad.v1);
            HashValue(hash, quad.textureId);
            HashValue(hash, quad.tint.kind);
            HashValue(hash, quad.tint.color);
            HashValue(hash, quad.opacity);
            HashClip(hash, quad.clip);
            HashValue(hash, quad.rotationRadians);
            HashValue(hash, quad.pivotX);
            HashValue(hash, quad.pivotY);
            break;
        }
        }
    }

    HashValue(hash, commands.clearColor);
    HashValue(hash, commands.hasClearColor);
    return hash;
}

NeoWPF::Contracts::IRendering* CurrentRendering() noexcept
{
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    return platform ? &platform->Rendering() : nullptr;
}

}

std::uint64_t NeoWPF::Rendering::CommandList::ComputeHash() const noexcept
{
    return HashCommandList(*this);
}

bool NeoWPF::Rendering::Renderer_Initialize(const RenderSurface& surface)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
        NeoWPF::Diagnostics::Level::Trace,
        "[Renderer/Surface] initialize request: handle=%p generation=%llu valid=%d",
        surface.nativeWindowHandle,
        static_cast<unsigned long long>(surface.GetGenerationId()), surface.IsValid() ? 1 : 0);
    auto surfaceUse = surface.TryAcquireUse();
    if (!surfaceUse.owns_lock()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Trace,
            "[Renderer/Surface] initialize rejected: handle=%p generation=%llu surface became unavailable",
            surface.nativeWindowHandle,
            static_cast<unsigned long long>(surface.GetGenerationId()));
        return false;
    }
    const auto existingState = s_windowStates.find(surface.nativeWindowHandle);
    if (existingState != s_windowStates.end() &&
        existingState->second.generationId != surface.GetGenerationId()) {
        if (existingState->second.surfaceCreated) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
                NeoWPF::Diagnostics::Level::Trace,
                "[Renderer/Surface] initialize blocked by live prior generation: handle=%p requestedGeneration=%llu existingGeneration=%llu",
                surface.nativeWindowHandle,
                static_cast<unsigned long long>(surface.GetGenerationId()),
                static_cast<unsigned long long>(existingState->second.generationId));
            return false;
        }
        s_windowStates.erase(existingState);
    }
    auto& state = GetWindowFrameState(surface.nativeWindowHandle);
    state.generationId = surface.GetGenerationId();
    if (state.surfaceCreated) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Trace,
            "[Renderer/Surface] already initialized: handle=%p generation=%llu",
            surface.nativeWindowHandle,
            static_cast<unsigned long long>(surface.GetGenerationId()));
        return true;
    }
    auto* rendering = CurrentRendering();
    if (!rendering) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Trace,
            "[Renderer/Surface] initialize failed: handle=%p generation=%llu no rendering backend",
            surface.nativeWindowHandle,
            static_cast<unsigned long long>(surface.GetGenerationId()));
        return false;
    }
    if (!rendering->IsInitialized() && !rendering->InitializeDevice(surface)) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Trace,
            "[Renderer/Surface] device initialization failed: handle=%p generation=%llu",
            surface.nativeWindowHandle,
            static_cast<unsigned long long>(surface.GetGenerationId()));
        return false;
    }
    state.surfaceCreated = rendering->CreateSurface(surface);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
        NeoWPF::Diagnostics::Level::Trace,
        "[Renderer/Surface] backend create result=%s: handle=%p generation=%llu backendInitialized=%d",
        state.surfaceCreated ? "complete" : "failed", surface.nativeWindowHandle,
        static_cast<unsigned long long>(surface.GetGenerationId()), rendering->IsInitialized() ? 1 : 0);
    return state.surfaceCreated;
}

void NeoWPF::Rendering::Renderer_DestroySurface(const RenderSurface& surface)
{
    if (!surface.nativeWindowHandle || !surface.generation) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Trace,
            "[Renderer/Surface] destroy ignored: handle=%p generation=%llu generationObject=%d",
            surface.nativeWindowHandle,
            static_cast<unsigned long long>(surface.GetGenerationId()), surface.generation ? 1 : 0);
        return;
    }
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
        NeoWPF::Diagnostics::Level::Trace,
        "[Renderer/Surface] destroy request: handle=%p generation=%llu valid=%d",
        surface.nativeWindowHandle,
        static_cast<unsigned long long>(surface.GetGenerationId()), surface.IsValid() ? 1 : 0);
    auto surfaceDestruction = surface.LockForDestruction();
    if (!surfaceDestruction.owns_lock()) {
        return;
    }
    const auto state = s_windowStates.find(surface.nativeWindowHandle);
    if (state == s_windowStates.end() || state->second.generationId != surface.GetGenerationId()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Trace,
            "[Renderer/Surface] destroy found no matching state: handle=%p generation=%llu stateFound=%d stateGeneration=%llu",
            surface.nativeWindowHandle,
            static_cast<unsigned long long>(surface.GetGenerationId()),
            state != s_windowStates.end() ? 1 : 0,
            state != s_windowStates.end()
                ? static_cast<unsigned long long>(state->second.generationId) : 0ull);
        return;
    }
    if (state->second.surfaceCreated) {
        if (auto* rendering = CurrentRendering()) {
            rendering->DestroySurface(surface);
        }
    }
    s_windowStates.erase(state);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
        NeoWPF::Diagnostics::Level::Trace,
        "[Renderer/Surface] destroy completed: handle=%p generation=%llu",
        surface.nativeWindowHandle,
        static_cast<unsigned long long>(surface.GetGenerationId()));
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
        NeoWPF::Diagnostics::Level::Trace,
        "[Renderer/Surface] resize request: handle=%p generation=%llu previousExtent=%ux%u extent=%ux%u",
        surface.nativeWindowHandle,
        static_cast<unsigned long long>(surface.GetGenerationId()),
        state->hasSize ? state->lastWidth : 0u, state->hasSize ? state->lastHeight : 0u, w, h);
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
    const auto commandHash = cmd.ComputeHash();
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
