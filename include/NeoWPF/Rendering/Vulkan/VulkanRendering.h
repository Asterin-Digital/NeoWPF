#pragma once

#include <NeoWPF/Contracts/IRendering.h>

#include <vulkan/vulkan.h>

#include <vector>

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
    void DestroySwapchainResources() noexcept;
    bool CreateSwapchainResources(unsigned int width, unsigned int height);

    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    std::uint32_t m_graphicsQueueFamily = 0;
    std::uint32_t m_presentQueueFamily = 0;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_swapchainFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_swapchainExtent{};
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_framebuffers;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;
    VkSemaphore m_imageAvailable = VK_NULL_HANDLE;
    VkSemaphore m_renderFinished = VK_NULL_HANDLE;
    VkFence m_inFlight = VK_NULL_HANDLE;
    std::uint32_t m_currentImage = 0;
    bool m_frameActive = false;
    bool m_initialized = false;
};

}
