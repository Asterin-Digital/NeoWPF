#include <NeoWPF/Rendering/Vulkan/VulkanRendering.h>

#include <NeoWPF/Contracts/IWindowing.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Rendering/Clipping.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/Vulkan/SurfaceAlpha.h>

#include <algorithm>
#include <any>
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

VulkanRendering::WindowContext* VulkanRendering::FindWindowContext(void* nativeWindowHandle) noexcept
{
    std::lock_guard<std::mutex> lock(m_windowContextsMutex);
    const auto iterator = m_windowContexts.find(nativeWindowHandle);
    return iterator == m_windowContexts.end() ? nullptr : iterator->second.get();
}

const VulkanRendering::WindowContext* VulkanRendering::FindWindowContext(void* nativeWindowHandle) const noexcept
{
    std::lock_guard<std::mutex> lock(m_windowContextsMutex);
    const auto iterator = m_windowContexts.find(nativeWindowHandle);
    return iterator == m_windowContexts.end() ? nullptr : iterator->second.get();
}

VulkanRendering::WindowContext* VulkanRendering::FirstWindowContext() noexcept
{
    std::lock_guard<std::mutex> lock(m_windowContextsMutex);
    return m_windowContexts.empty() ? nullptr : m_windowContexts.begin()->second.get();
}

const VulkanRendering::WindowContext* VulkanRendering::FirstWindowContext() const noexcept
{
    std::lock_guard<std::mutex> lock(m_windowContextsMutex);
    return m_windowContexts.empty() ? nullptr : m_windowContexts.begin()->second.get();
}

bool VulkanRendering::InitializeDevice(const NeoWPF::Rendering::RenderSurface& surface)
{
    std::lock_guard<std::recursive_mutex> renderLock(m_renderMutex);
    void* nativeWindowHandle = surface.nativeWindowHandle;
    if (!nativeWindowHandle) {
        return false;
    }
    if (m_initialized) {
        return true;
    }

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

    VkSurfaceKHR compatibilitySurface = VK_NULL_HANDLE;
    NeoWPF::Diagnostics::LogInfo("[Vulkan] Creating temporary surface for device selection");
    const bool compatibilitySurfaceCreated = windowInfo.createVulkanSurface(
            windowInfo.vulkanSurfaceUserData, m_instance, windowInfo.window,
            windowInfo.display, &compatibilitySurface);
    if (!compatibilitySurfaceCreated || compatibilitySurface == VK_NULL_HANDLE) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Temporary surface creation failed");
        if (compatibilitySurface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(m_instance, compatibilitySurface, nullptr);
        }
        ShutdownDevice();
        return false;
    }

    if (!InitializeDeviceForSurface(compatibilitySurface)) {
        if (compatibilitySurface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(m_instance, compatibilitySurface, nullptr);
        }
        ShutdownDevice();
        return false;
    }
    vkDestroySurfaceKHR(m_instance, compatibilitySurface, nullptr);
    m_initialized = true;
    NeoWPF::Diagnostics::LogInfo("[Vulkan] Device initialization complete");
    return true;
}

bool VulkanRendering::CreateSurface(const NeoWPF::Rendering::RenderSurface& surface)
{
    std::lock_guard<std::recursive_mutex> renderLock(m_renderMutex);
    if (!m_initialized || !surface.nativeWindowHandle) {
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(m_windowContextsMutex);
        if (m_windowContexts.find(surface.nativeWindowHandle) != m_windowContexts.end()) {
            return true;
        }
    }

    NeoWPF::Contracts::NativeWindowInfo windowInfo;
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform || !platform->Windowing().GetNativeWindowInfo(surface.nativeWindowHandle, windowInfo) ||
        !windowInfo.window || !windowInfo.createVulkanSurface) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Invalid surface factory for rendering surface");
        return false;
    }

    auto context = std::make_shared<WindowContext>();
    context->transparent = windowInfo.surfaceAlpha == NeoWPF::Contracts::SurfaceAlpha::Premultiplied;
    const bool nativeSurfaceCreated = windowInfo.createVulkanSurface(
            windowInfo.vulkanSurfaceUserData, m_instance, windowInfo.window,
            windowInfo.display, &context->surface);
    if (!nativeSurfaceCreated || context->surface == VK_NULL_HANDLE) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Platform surface creation failed");
        if (context->surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(m_instance, context->surface, nullptr);
        }
        return false;
    }
    if (!CreateSwapchainResources(*context, 1, 1)) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Swapchain resource creation failed for rendering surface");
        DestroySwapchainResources(*context);
        vkDestroySurfaceKHR(m_instance, context->surface, nullptr);
        return false;
    }
    std::lock_guard<std::mutex> lock(m_windowContextsMutex);
    m_windowContexts.emplace(surface.nativeWindowHandle, std::move(context));
    return true;
}

void VulkanRendering::DestroySurface(const NeoWPF::Rendering::RenderSurface& surface) noexcept
{
    std::lock_guard<std::recursive_mutex> renderLock(m_renderMutex);
    std::shared_ptr<WindowContext> context;
    {
        std::lock_guard<std::mutex> lock(m_windowContextsMutex);
        const auto iterator = m_windowContexts.find(surface.nativeWindowHandle);
        if (iterator == m_windowContexts.end()) {
            return;
        }
        context = std::move(iterator->second);
        m_windowContexts.erase(iterator);
    }
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
    }
    DestroySwapchainResources(*context);
    if (context->surface != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, context->surface, nullptr);
        context->surface = VK_NULL_HANDLE;
    }
}

void VulkanRendering::ShutdownDevice() noexcept
{
    std::lock_guard<std::recursive_mutex> renderLock(m_renderMutex);
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
    }
    DestroyTextureResources();
    {
        std::lock_guard<std::mutex> contextLock(m_windowContextsMutex);
        for (const auto& [handle, context] : m_windowContexts) {
            (void)handle;
            DestroySwapchainResources(*context);
            if (context->surface != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE) {
                vkDestroySurfaceKHR(m_instance, context->surface, nullptr);
            }
        }
        m_windowContexts.clear();
    }
    if (m_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }
    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
    std::lock_guard<std::mutex> contextLock(m_windowContextsMutex);
    m_windowContexts.clear();
    m_physicalDevice = VK_NULL_HANDLE;
    m_initialized = false;
}

bool VulkanRendering::IsInitialized() const noexcept { return m_initialized; }

void VulkanRendering::Resize(const NeoWPF::Rendering::RenderSurface& surface,
                             unsigned int width, unsigned int height) noexcept
{
    std::lock_guard<std::recursive_mutex> renderLock(m_renderMutex);
    auto* context = FindWindowContext(surface.nativeWindowHandle);
    if (m_initialized && context && width > 0 && height > 0 &&
        (context->swapchainExtent.width != width || context->swapchainExtent.height != height)) {
        vkDeviceWaitIdle(m_device);
        if (!CreateSwapchainResources(*context, width, height)) {
            NeoWPF::Diagnostics::LogError("[Vulkan] Resize failed: swapchain resources are unavailable");
            context->swapchainExtent = {};
            context->frameActive = false;
        }
    }
}

void VulkanRendering::BeginFrame(const NeoWPF::Rendering::RenderSurface& surface)
{
    std::lock_guard<std::recursive_mutex> renderLock(m_renderMutex);
    auto* context = FindWindowContext(surface.nativeWindowHandle);
    if (!m_initialized || !context || context->swapchainExtent.width == 0 || context->swapchainExtent.height == 0) {
        return;
    }
    VkResult result = vkWaitForFences(m_device, 1, &context->inFlight, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] vkWaitForFences failed: %s (%d)",
            VkResultName(result), static_cast<int>(result));
        return;
    }
    result = vkAcquireNextImageKHR(
        m_device, context->swapchain, UINT64_MAX, context->imageAvailable, VK_NULL_HANDLE, &context->currentImage);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        NeoWPF::Diagnostics::LogError("[Vulkan] vkAcquireNextImageKHR failed: %s (%d)",
            VkResultName(result), static_cast<int>(result));
        return;
    }
    if (result == VK_SUBOPTIMAL_KHR) {
        NeoWPF::Diagnostics::LogWarn("[Vulkan] vkAcquireNextImageKHR returned VK_SUBOPTIMAL_KHR");
    }
    result = vkResetFences(m_device, 1, &context->inFlight);
    if (result != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] vkResetFences failed: %s (%d)",
            VkResultName(result), static_cast<int>(result));
        return;
    }
    result = vkResetCommandBuffer(context->commandBuffers[context->currentImage], 0);
    if (result != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] vkResetCommandBuffer failed: %s (%d)",
            VkResultName(result), static_cast<int>(result));
        return;
    }
    const VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 0, nullptr};
    result = vkBeginCommandBuffer(context->commandBuffers[context->currentImage], &beginInfo);
    if (result != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] vkBeginCommandBuffer failed: %s (%d)",
            VkResultName(result), static_cast<int>(result));
        return;
    }
    const VkClearValue clearValue{{0.0f, 0.0f, 0.0f, 1.0f}};
    const VkRenderPassBeginInfo renderPassInfo{
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, context->renderPass,
        context->framebuffers[context->currentImage], {{0, 0}, context->swapchainExtent}, 1, &clearValue};
    vkCmdBeginRenderPass(context->commandBuffers[context->currentImage], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    context->frameActive = true;
}

void VulkanRendering::Present(const NeoWPF::Rendering::RenderSurface& surface)
{
    std::lock_guard<std::recursive_mutex> renderLock(m_renderMutex);
    auto* context = FindWindowContext(surface.nativeWindowHandle);
    if (!context || !context->frameActive) {
        return;
    }
    vkCmdEndRenderPass(context->commandBuffers[context->currentImage]);
    VkResult result = vkEndCommandBuffer(context->commandBuffers[context->currentImage]);
    if (result != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] vkEndCommandBuffer failed: %s (%d)",
            VkResultName(result), static_cast<int>(result));
        context->frameActive = false;
        return;
    }
    const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    const VkSubmitInfo submitInfo{
        VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &context->imageAvailable, &waitStage,
        1, &context->commandBuffers[context->currentImage], 1, &context->renderFinished};
    result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, context->inFlight);
    if (result != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] vkQueueSubmit failed: %s (%d)",
            VkResultName(result), static_cast<int>(result));
        context->frameActive = false;
        return;
    }
    const VkPresentInfoKHR presentInfo{
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, nullptr, 1, &context->renderFinished,
        1, &context->swapchain, &context->currentImage, nullptr};
    result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        NeoWPF::Diagnostics::LogError("[Vulkan] vkQueuePresentKHR failed: %s (%d)",
            VkResultName(result), static_cast<int>(result));
    } else if (result == VK_SUBOPTIMAL_KHR) {
        NeoWPF::Diagnostics::LogWarn("[Vulkan] vkQueuePresentKHR returned VK_SUBOPTIMAL_KHR");
    }
    context->frameActive = false;
}

}
