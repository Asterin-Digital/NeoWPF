#include <NeoWPF/Rendering/Vulkan/VulkanRendering.h>

#include <NeoWPF/Contracts/IWindowing.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Rendering/Renderer.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <vector>

namespace NeoWPF::Rendering::Vulkan {
namespace {

bool HasExtension(const std::vector<VkExtensionProperties>& extensions, const char* name)
{
    return std::any_of(extensions.begin(), extensions.end(), [name](const auto& extension) {
        return std::strcmp(extension.extensionName, name) == 0;
    });
}

}

bool VulkanRendering::Initialize(void* nativeWindowHandle)
{
    if (m_initialized) {
        return true;
    }

    NeoWPF::Contracts::NativeWindowInfo windowInfo;
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform || !platform->Windowing().GetNativeWindowInfo(nativeWindowHandle, windowInfo) ||
        !windowInfo.window || !windowInfo.createVulkanSurface) {
        NeoWPF::Diagnostics::LogError("Vulkan initialization requires a valid platform surface factory");
        return false;
    }

    std::uint32_t extensionCount = 0;
    if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("Vulkan failed to enumerate instance extensions");
        return false;
    }
    std::vector<VkExtensionProperties> extensions(extensionCount);
    if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()) != VK_SUCCESS ||
        !HasExtension(extensions, VK_KHR_SURFACE_EXTENSION_NAME)) {
        NeoWPF::Diagnostics::LogError("Vulkan surface extension is unavailable");
        return false;
    }

    const VkApplicationInfo applicationInfo{
        VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, "NeoWPF", VK_MAKE_VERSION(1, 0, 0),
        "NeoWPF Vulkan backend", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0};
    std::vector<const char*> instanceExtensions;
    instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    for (std::size_t index = 0; index < windowInfo.vulkanInstanceExtensionCount; ++index) {
        if (windowInfo.vulkanInstanceExtensions[index] &&
            HasExtension(extensions, windowInfo.vulkanInstanceExtensions[index])) {
            instanceExtensions.push_back(windowInfo.vulkanInstanceExtensions[index]);
        } else {
            NeoWPF::Diagnostics::LogError("Required platform Vulkan instance extension is unavailable");
            return false;
        }
    }
    const VkInstanceCreateInfo instanceCreateInfo{
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, &applicationInfo,
        static_cast<std::uint32_t>(instanceExtensions.size()), instanceExtensions.data(), 0, nullptr};
    const VkResult instanceResult = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
    if (instanceResult != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("Vulkan instance creation failed: %d", static_cast<int>(instanceResult));
        return false;
    }

    if (!windowInfo.createVulkanSurface(
            windowInfo.vulkanSurfaceUserData, m_instance, windowInfo.window,
            windowInfo.display, &m_surface)) {
        NeoWPF::Diagnostics::LogError("Platform Vulkan surface creation failed");
        Shutdown();
        return false;
    }

    std::uint32_t deviceCount = 0;
    if (vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr) != VK_SUCCESS || deviceCount == 0) {
        NeoWPF::Diagnostics::LogError("Vulkan found no physical devices");
        Shutdown();
        return false;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    for (auto device : devices) {
        std::uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);
        std::vector<VkQueueFamilyProperties> queues(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, queues.data());
        std::uint32_t graphicsFamily = std::numeric_limits<std::uint32_t>::max();
        std::uint32_t presentFamily = std::numeric_limits<std::uint32_t>::max();
        for (std::uint32_t index = 0; index < queueCount; ++index) {
            if (queues[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsFamily = index;
            }
            VkBool32 presentSupported = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, index, m_surface, &presentSupported);
            if (presentSupported) {
                presentFamily = index;
            }
        }
        if (graphicsFamily != std::numeric_limits<std::uint32_t>::max() &&
            presentFamily != std::numeric_limits<std::uint32_t>::max()) {
            m_physicalDevice = device;
            m_graphicsQueueFamily = graphicsFamily;
            m_presentQueueFamily = presentFamily;
            break;
        }
    }
    if (m_physicalDevice == VK_NULL_HANDLE) {
        NeoWPF::Diagnostics::LogError("Vulkan found no device with graphics and Wayland presentation support");
        Shutdown();
        return false;
    }

    const float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    queueInfos.push_back({VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0,
                          m_graphicsQueueFamily, 1, &queuePriority});
    if (m_presentQueueFamily != m_graphicsQueueFamily) {
        queueInfos.push_back({VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0,
                              m_presentQueueFamily, 1, &queuePriority});
    }
    const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const VkDeviceCreateInfo deviceCreateInfo{
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0,
        static_cast<std::uint32_t>(queueInfos.size()), queueInfos.data(),
        0, nullptr, 1, deviceExtensions, nullptr};
    const VkResult deviceResult = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
    if (deviceResult != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("Vulkan logical device creation failed: %d", static_cast<int>(deviceResult));
        Shutdown();
        return false;
    }
    vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_presentQueueFamily, 0, &m_presentQueue);

    if (!CreateSwapchainResources(1, 1)) {
        NeoWPF::Diagnostics::LogError("Vulkan swapchain resource creation failed");
        Shutdown();
        return false;
    }
    m_initialized = true;
    return true;
}

void VulkanRendering::Shutdown() noexcept
{
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
    }
    DestroySwapchainResources();
    if (m_surface != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
    if (m_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }
    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
    m_physicalDevice = VK_NULL_HANDLE;
    m_initialized = false;
}

bool VulkanRendering::IsInitialized() const noexcept { return m_initialized; }

void VulkanRendering::Resize(unsigned int width, unsigned int height) noexcept
{
    if (m_initialized && width > 0 && height > 0 &&
        (m_swapchainExtent.width != width || m_swapchainExtent.height != height)) {
        vkDeviceWaitIdle(m_device);
        CreateSwapchainResources(width, height);
    }
}

void VulkanRendering::BeginFrame()
{
    if (!m_initialized || m_swapchainExtent.width == 0 || m_swapchainExtent.height == 0) {
        return;
    }
    vkWaitForFences(m_device, 1, &m_inFlight, VK_TRUE, UINT64_MAX);
    const VkResult result = vkAcquireNextImageKHR(
        m_device, m_swapchain, UINT64_MAX, m_imageAvailable, VK_NULL_HANDLE, &m_currentImage);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        return;
    }
    vkResetFences(m_device, 1, &m_inFlight);
    vkResetCommandBuffer(m_commandBuffers[m_currentImage], 0);
    const VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 0, nullptr};
    vkBeginCommandBuffer(m_commandBuffers[m_currentImage], &beginInfo);
    const VkClearValue clearValue{{0.08f, 0.10f, 0.14f, 1.0f}};
    const VkRenderPassBeginInfo renderPassInfo{
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, m_renderPass,
        m_framebuffers[m_currentImage], {{0, 0}, m_swapchainExtent}, 1, &clearValue};
    vkCmdBeginRenderPass(m_commandBuffers[m_currentImage], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_frameActive = true;
}

void VulkanRendering::Execute(const NeoWPF::Rendering::CommandList&) {}

void VulkanRendering::Present()
{
    if (!m_frameActive) {
        return;
    }
    vkCmdEndRenderPass(m_commandBuffers[m_currentImage]);
    vkEndCommandBuffer(m_commandBuffers[m_currentImage]);
    const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    const VkSubmitInfo submitInfo{
        VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &m_imageAvailable, &waitStage,
        1, &m_commandBuffers[m_currentImage], 1, &m_renderFinished};
    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlight);
    const VkPresentInfoKHR presentInfo{
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, nullptr, 1, &m_renderFinished,
        1, &m_swapchain, &m_currentImage, nullptr};
    vkQueuePresentKHR(m_presentQueue, &presentInfo);
    m_frameActive = false;
}

std::uint32_t VulkanRendering::CreateTexture(unsigned int, unsigned int, const void*) { return 0; }
void VulkanRendering::DestroyTexture(std::uint32_t) noexcept {}
void VulkanRendering::ClearTextures() noexcept {}

}
