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

const char* VkResultName(VkResult result)
{
    switch (result) {
    case VK_SUCCESS: return "VK_SUCCESS";
    case VK_NOT_READY: return "VK_NOT_READY";
    case VK_TIMEOUT: return "VK_TIMEOUT";
    case VK_INCOMPLETE: return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
    default: return "VK_RESULT_UNKNOWN";
    }
}

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

    NeoWPF::Diagnostics::LogInfo("[Vulkan] Initialize begin: nativeWindowHandle=%p", nativeWindowHandle);
    NeoWPF::Contracts::NativeWindowInfo windowInfo;
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform || !platform->Windowing().GetNativeWindowInfo(nativeWindowHandle, windowInfo) ||
        !windowInfo.window || !windowInfo.createVulkanSurface) {
        NeoWPF::Diagnostics::LogError(
            "[Vulkan] Invalid surface factory: platform=%p window=%p display=%p callback=%p",
            platform, windowInfo.window, windowInfo.display,
            reinterpret_cast<void*>(windowInfo.createVulkanSurface));
        return false;
    }
    NeoWPF::Diagnostics::LogInfo(
        "[Vulkan] Native window info: system=%d window=%p display=%p requestedExtensions=%zu",
        static_cast<int>(windowInfo.system), windowInfo.window, windowInfo.display,
        windowInfo.vulkanInstanceExtensionCount);

    std::uint32_t extensionCount = 0;
    VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    if (result != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Instance extension count query failed: %s (%d)",
                                      VkResultName(result), static_cast<int>(result));
        return false;
    }
    std::vector<VkExtensionProperties> extensions(extensionCount);
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    if (result != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Instance extension list query failed: %s (%d)",
                                      VkResultName(result), static_cast<int>(result));
        return false;
    }
    NeoWPF::Diagnostics::LogInfo("[Vulkan] Available instance extensions: %u", extensionCount);
    for (const auto& extension : extensions) {
        NeoWPF::Diagnostics::LogInfo("[Vulkan] Available extension: %s", extension.extensionName);
    }
    if (!HasExtension(extensions, VK_KHR_SURFACE_EXTENSION_NAME)) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Required extension unavailable: %s", VK_KHR_SURFACE_EXTENSION_NAME);
        return false;
    }

    const VkApplicationInfo applicationInfo{
        VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, "NeoWPF", VK_MAKE_VERSION(1, 0, 0),
        "NeoWPF Vulkan backend", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0};
    std::vector<const char*> instanceExtensions;
    instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    for (std::size_t index = 0; index < windowInfo.vulkanInstanceExtensionCount; ++index) {
        NeoWPF::Diagnostics::LogInfo("[Vulkan] Checking platform extension: %s",
            windowInfo.vulkanInstanceExtensions[index] ? windowInfo.vulkanInstanceExtensions[index] : "<null>");
        if (windowInfo.vulkanInstanceExtensions[index] &&
            HasExtension(extensions, windowInfo.vulkanInstanceExtensions[index])) {
            instanceExtensions.push_back(windowInfo.vulkanInstanceExtensions[index]);
        } else {
            NeoWPF::Diagnostics::LogError("[Vulkan] Required platform instance extension unavailable: %s",
                windowInfo.vulkanInstanceExtensions[index] ? windowInfo.vulkanInstanceExtensions[index] : "<null>");
            return false;
        }
    }
    const VkInstanceCreateInfo instanceCreateInfo{
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        nullptr,
        0,
        &applicationInfo,
        0,
        nullptr,
        static_cast<std::uint32_t>(instanceExtensions.size()),
        instanceExtensions.data()
    };

    NeoWPF::Diagnostics::LogInfo(
        "[Vulkan] Creating instance: extensions=%u layers=%u apiVersion=%u.%u.%u",
        instanceCreateInfo.enabledExtensionCount,
        instanceCreateInfo.enabledLayerCount,
        VK_VERSION_MAJOR(applicationInfo.apiVersion), VK_VERSION_MINOR(applicationInfo.apiVersion),
        VK_VERSION_PATCH(applicationInfo.apiVersion));

    for (std::uint32_t i = 0; i < instanceCreateInfo.enabledExtensionCount; ++i) {
        NeoWPF::Diagnostics::LogInfo(
            "[Vulkan] Enabled instance extension[%u]: %s",
            i,
            instanceCreateInfo.ppEnabledExtensionNames[i]);
    }

    const VkResult instanceResult = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
    if (instanceResult != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] vkCreateInstance failed: %s (%d)",
            VkResultName(instanceResult), static_cast<int>(instanceResult));
        return false;
    }
    NeoWPF::Diagnostics::LogInfo("[Vulkan] Instance created: %p", static_cast<void*>(m_instance));

    NeoWPF::Diagnostics::LogInfo("[Vulkan] Creating platform surface");
    if (!windowInfo.createVulkanSurface(
            windowInfo.vulkanSurfaceUserData, m_instance, windowInfo.window,
            windowInfo.display, &m_surface)) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Platform surface creation failed");
        Shutdown();
        return false;
    }
    NeoWPF::Diagnostics::LogInfo("[Vulkan] Surface created: %p", static_cast<void*>(m_surface));

    std::uint32_t deviceCount = 0;
    result = vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    if (result != VK_SUCCESS || deviceCount == 0) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Physical device count query failed: %s (%d), count=%u",
            VkResultName(result), static_cast<int>(result), deviceCount);
        Shutdown();
        return false;
    }
    NeoWPF::Diagnostics::LogInfo("[Vulkan] Physical devices found: %u", deviceCount);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    result = vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    if (result != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Physical device list query failed: %s (%d)",
            VkResultName(result), static_cast<int>(result));
        Shutdown();
        return false;
    }
    for (std::uint32_t deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
        const auto device = devices[deviceIndex];
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device, &properties);
        NeoWPF::Diagnostics::LogInfo("[Vulkan] Device[%u]: name='%s' type=%d apiVersion=%u.%u.%u",
            deviceIndex, properties.deviceName, static_cast<int>(properties.deviceType),
            VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion),
            VK_VERSION_PATCH(properties.apiVersion));
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
            const VkResult presentResult = vkGetPhysicalDeviceSurfaceSupportKHR(
                device, index, m_surface, &presentSupported);
            NeoWPF::Diagnostics::LogInfo("[Vulkan] Device[%u] queue[%u]: flags=0x%X graphics=%s present=%s result=%s",
                deviceIndex, index, queues[index].queueFlags,
                (queues[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) ? "yes" : "no",
                presentSupported ? "yes" : "no", VkResultName(presentResult));
            if (presentSupported) {
                presentFamily = index;
            }
        }
        if (graphicsFamily != std::numeric_limits<std::uint32_t>::max() &&
            presentFamily != std::numeric_limits<std::uint32_t>::max()) {
            m_physicalDevice = device;
            m_graphicsQueueFamily = graphicsFamily;
            m_presentQueueFamily = presentFamily;
            NeoWPF::Diagnostics::LogInfo("[Vulkan] Selected device[%u]: graphicsQueueFamily=%u presentQueueFamily=%u",
                deviceIndex, m_graphicsQueueFamily, m_presentQueueFamily);
            break;
        }
    }
    if (m_physicalDevice == VK_NULL_HANDLE) {
        NeoWPF::Diagnostics::LogError("[Vulkan] No device has graphics and surface presentation support");
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
        NeoWPF::Diagnostics::LogError("[Vulkan] vkCreateDevice failed: %s (%d)",
            VkResultName(deviceResult), static_cast<int>(deviceResult));
        Shutdown();
        return false;
    }
    NeoWPF::Diagnostics::LogInfo("[Vulkan] Logical device created: %p", static_cast<void*>(m_device));
    vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_presentQueueFamily, 0, &m_presentQueue);

    if (!CreateSwapchainResources(1, 1)) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Swapchain resource creation failed during initialization");
        Shutdown();
        return false;
    }
    m_initialized = true;
    NeoWPF::Diagnostics::LogInfo("[Vulkan] Initialize complete");
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

void VulkanRendering::Execute(const NeoWPF::Rendering::CommandList&) 
{
    // @todo make this
}

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

void VulkanRendering::DestroyTexture(std::uint32_t) noexcept 
{
    // @todo make this
}

void VulkanRendering::ClearTextures() noexcept 
{
    // @todo make this
}

}
