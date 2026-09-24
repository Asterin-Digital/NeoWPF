/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>

namespace NeoWPF::Rendering {
struct CommandList;
}

namespace NeoWPF::Contracts {

class IRendering {
public:
    virtual ~IRendering() = default;

    virtual bool Initialize(void* nativeWindowHandle) = 0;
    virtual void Shutdown() noexcept = 0;
    virtual bool IsInitialized() const noexcept = 0;
    virtual void Resize(unsigned int width, unsigned int height) noexcept = 0;
    virtual void BeginFrame() = 0;
    virtual void Execute(const NeoWPF::Rendering::CommandList& commands) = 0;
    virtual void Present() = 0;
    virtual std::uint32_t CreateTexture(unsigned int width, unsigned int height, const void* rgbaData) = 0;
    virtual void DestroyTexture(std::uint32_t id) noexcept = 0;
    virtual void ClearTextures() noexcept = 0;
};

}
