/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IRendering.h>

namespace NeoWPF::Rendering::D3D11 {

class WRendering final : public NeoWPF::Contracts::IRendering {
public:
    bool Initialize(const NeoWPF::Rendering::RenderSurface& surface) override;
    void Shutdown() noexcept override;
    bool IsInitialized() const noexcept override;
    void Resize(const NeoWPF::Rendering::RenderSurface& surface,
                unsigned int width, unsigned int height) noexcept override;
    void BeginFrame(const NeoWPF::Rendering::RenderSurface& surface) override;
    void Execute(const NeoWPF::Rendering::RenderSurface& surface,
                 const NeoWPF::Rendering::CommandList& commands) override;
    void Present(const NeoWPF::Rendering::RenderSurface& surface) override;
    std::uint32_t CreateTexture(unsigned int width, unsigned int height, const void* rgbaData) override;
    void DestroyTexture(std::uint32_t id) noexcept override;
    void ClearTextures() noexcept override;
};

}
