#pragma once

#include <NeoWPF/Contracts/IRendering.h>

#include <vulkan/vulkan.h>

namespace NeoWPF::Rendering::Vulkan {

class VulkanRendering final : public NeoWPF::Contracts::IRendering {
public:
    bool Initialize(void* nativeWindowHandle) override;
    void Shutdown() noexcept override;
    bool IsInitialized() const noexcept override;
    void Resize(unsigned int width, unsigned int height) noexcept override;
    void BeginFrame() override;
    void Execute(const NeoWPF::Rendering::CommandList& commands) override;
    void Present() override;
    std::uint32_t CreateTexture(unsigned int width, unsigned int height, const void* rgbaData) override;
    void DestroyTexture(std::uint32_t id) noexcept override;
    void ClearTextures() noexcept override;

private:
    VkInstance m_instance = VK_NULL_HANDLE;
    bool m_initialized = false;
};

}
