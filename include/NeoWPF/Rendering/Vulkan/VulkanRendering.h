/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

#pragma once

#include <NeoWPF/Contracts/IRendering.h>

#include <vulkan/vulkan.h>

#include <cstddef>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <memory>

namespace NeoWPF::Rendering::Vulkan {

class VulkanRendering final : public NeoWPF::Contracts::IRendering {
public:
    bool InitializeDevice(const NeoWPF::Rendering::RenderSurface& surface) override;
    bool CreateSurface(const NeoWPF::Rendering::RenderSurface& surface) override;
    void DestroySurface(const NeoWPF::Rendering::RenderSurface& surface) noexcept override;
    void ShutdownDevice() noexcept override;
    bool IsInitialized() const noexcept override;
    void Resize(const NeoWPF::Rendering::RenderSurface& surface,
                unsigned int width, unsigned int height) noexcept override;
    void BeginFrame(const NeoWPF::Rendering::RenderSurface& surface) override;
    void Execute(const NeoWPF::Rendering::RenderSurface& surface,
                 const NeoWPF::Rendering::CommandList& commands,
                 double scaleFactor) override;
    void Present(const NeoWPF::Rendering::RenderSurface& surface) override;
    void PrepareGradientAtlas(const NeoWPF::Rendering::CommandList& commands) override;
    std::uint32_t CreateTexture(unsigned int width, unsigned int height, const void* rgbaData) override;
    void DestroyTexture(std::uint32_t id) noexcept override;
    void ClearTextures() noexcept override;

private:
    struct GradientPushConstants {
        float viewport[2];
        float padding[2];
        float mapping0[4];
        float mapping1[4];
        float gradient[4];
    };

    static_assert(sizeof(GradientPushConstants) == 64);
    static_assert(offsetof(GradientPushConstants, mapping0) == 16);
    static_assert(offsetof(GradientPushConstants, mapping1) == 32);
    static_assert(offsetof(GradientPushConstants, gradient) == 48);

    struct VertexBuffer {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkDeviceSize capacity = 0;
    };

    struct WindowContext {
        VertexBuffer solidBuffer;
        VertexBuffer texturedBuffer;
        VertexBuffer gradientBuffer;
        VkPipelineLayout solidPipelineLayout = VK_NULL_HANDLE;
        VkPipeline solidPipeline = VK_NULL_HANDLE;
        VkPipelineLayout texturedPipelineLayout = VK_NULL_HANDLE;
        VkPipeline texturedPipeline = VK_NULL_HANDLE;
        VkPipelineLayout gradientPipelineLayout = VK_NULL_HANDLE;
        VkPipeline gradientPipeline = VK_NULL_HANDLE;
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
        bool hasPresentedFrame = false;
        bool transparent = false;
    };

    bool InitializeDeviceForSurface(VkSurfaceKHR surface);
    WindowContext* FindWindowContext(void* nativeWindowHandle) noexcept;
    const WindowContext* FindWindowContext(void* nativeWindowHandle) const noexcept;
    WindowContext* FirstWindowContext() noexcept;
    const WindowContext* FirstWindowContext() const noexcept;
    void DestroySwapchainResources(WindowContext&) noexcept;

    /*
        @summary
        Recreates a window's swapchain after an acquired frame cannot be submitted.

        @internal
    */
    void RecreateSwapchain(const NeoWPF::Rendering::RenderSurface&, WindowContext&) noexcept;

    void DestroySolidPipeline(WindowContext&) noexcept;
    bool CreateSolidPipeline(WindowContext&, VkRenderPass renderPass);
    void DestroyTexturedPipeline(WindowContext&) noexcept;
    void DestroyTextureResources() noexcept;
    bool CreateTexturedPipeline(WindowContext&, VkRenderPass renderPass);
    void DestroyGradientPipeline(WindowContext&) noexcept;
    bool CreateGradientPipeline(WindowContext&, VkRenderPass renderPass);
    bool CreateSwapchainResources(WindowContext&, unsigned int width, unsigned int height);

    /*
        @summary
        Grows a window-owned vertex buffer to fit the requested data.

        @param buffer
        The vertex buffer to grow.

        @param requiredSize
        The required byte capacity.

        @returns
        True when the buffer can hold the data.

        @internal
    */
    bool EnsureVertexBuffer(VertexBuffer& buffer, VkDeviceSize requiredSize);

    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    std::uint32_t m_graphicsQueueFamily = 0;
    std::uint32_t m_presentQueueFamily = 0;
    VkDescriptorSetLayout m_textureSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_textureDescriptorPool = VK_NULL_HANDLE;
    VkSampler m_textureSampler = VK_NULL_HANDLE;
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
    std::uint32_t m_gradientTextureId = 0;
    std::uint64_t m_gradientTextureHash = 0;
    std::unordered_map<void*, std::shared_ptr<WindowContext>> m_windowContexts;
    mutable std::mutex m_windowContextsMutex;
    bool m_initialized = false;
};

}
