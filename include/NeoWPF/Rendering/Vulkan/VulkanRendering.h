/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IRendering.h>

#include <vulkan/vulkan.h>

#include <mutex>
#include <unordered_map>
#include <vector>
#include <memory>

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
    struct WindowContext {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D swapchainExtent{};
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> framebuffers;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> commandBuffers;
        VkSemaphore imageAvailable = VK_NULL_HANDLE;
        VkSemaphore renderFinished = VK_NULL_HANDLE;
        VkFence inFlight = VK_NULL_HANDLE;
        std::uint32_t currentImage = 0;
        bool frameActive = false;
    };

    bool InitializeDevice();
    WindowContext* CurrentWindowContext() noexcept;
    const WindowContext* CurrentWindowContext() const noexcept;
    void DestroySwapchainResources(WindowContext&) noexcept;
    void DestroySolidPipeline() noexcept;
    bool CreateSolidPipeline();
    void DestroyTexturedPipeline() noexcept;
    void DestroyTextureResources() noexcept;
    bool CreateTexturedPipeline();
    bool CreateSwapchainResources(WindowContext&, unsigned int width, unsigned int height);

    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    std::uint32_t m_graphicsQueueFamily = 0;
    std::uint32_t m_presentQueueFamily = 0;
    VkPipelineLayout m_solidPipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_solidPipeline = VK_NULL_HANDLE;
    VkBuffer m_solidVertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_solidVertexMemory = VK_NULL_HANDLE;
    VkPipelineLayout m_texturedPipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_texturedPipeline = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_textureSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_textureDescriptorPool = VK_NULL_HANDLE;
    VkSampler m_textureSampler = VK_NULL_HANDLE;
    VkBuffer m_texturedVertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_texturedVertexMemory = VK_NULL_HANDLE;

    struct TextureResource {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkDescriptorSet descriptor = VK_NULL_HANDLE;
    };
    std::unordered_map<std::uint32_t, TextureResource> m_textures;
    std::mutex m_textureMutex;
    std::recursive_mutex m_renderMutex;
    std::uint32_t m_nextTextureId = 1;
    std::unordered_map<void*, std::shared_ptr<WindowContext>> m_windowContexts;
    mutable std::mutex m_windowContextsMutex;
    bool m_initialized = false;
};

}
