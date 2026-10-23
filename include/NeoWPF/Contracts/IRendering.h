/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <NeoWPF/Rendering/RenderSurface.h>

namespace NeoWPF::Rendering {
struct CommandList;
}

namespace NeoWPF::Contracts {

class IRendering {
public:
    virtual ~IRendering() = default;

    virtual bool Initialize(const NeoWPF::Rendering::RenderSurface& surface) = 0;
    virtual void Shutdown() noexcept = 0;
    virtual bool IsInitialized() const noexcept = 0;
    virtual void Resize(const NeoWPF::Rendering::RenderSurface& surface,
                        unsigned int width, unsigned int height) noexcept = 0;
    virtual void BeginFrame(const NeoWPF::Rendering::RenderSurface& surface) = 0;
    virtual void Execute(const NeoWPF::Rendering::RenderSurface& surface,
                         const NeoWPF::Rendering::CommandList& commands) = 0;
    virtual void Present(const NeoWPF::Rendering::RenderSurface& surface) = 0;
    virtual std::uint32_t CreateTexture(unsigned int width, unsigned int height, const void* rgbaData) = 0;
    virtual void DestroyTexture(std::uint32_t id) noexcept = 0;
    virtual void ClearTextures() noexcept = 0;
};

}
