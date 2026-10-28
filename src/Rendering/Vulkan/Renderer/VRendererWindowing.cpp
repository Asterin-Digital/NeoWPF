#include <NeoWPF/Rendering/Vulkan/VulkanRendering.h>
#include <NeoWPF/Rendering/Vulkan/SurfaceAlpha.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <algorithm>
#include <array>
#include <limits>
#include <vector>

namespace NeoWPF::Rendering::Vulkan {

void VulkanRendering::DestroySwapchainResources(WindowContext& context) noexcept
{
    if (m_device == VK_NULL_HANDLE) {
        return;
    }
    DestroyTexturedPipeline();
    DestroySolidPipeline();
    for (auto framebuffer : context.framebuffers) {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
    context.framebuffers.clear();
    if (context.renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_device, context.renderPass, nullptr);
    }
    context.renderPass = VK_NULL_HANDLE;
    for (auto view : context.swapchainImageViews) {
        vkDestroyImageView(m_device, view, nullptr);
    }
    context.swapchainImageViews.clear();
    if (context.swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_device, context.swapchain, nullptr);
    }
    context.swapchain = VK_NULL_HANDLE;
    if (context.commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device, context.commandPool, nullptr);
    }
    context.commandPool = VK_NULL_HANDLE;
    if (context.imageAvailable != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, context.imageAvailable, nullptr);
    }
    if (context.renderFinished != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, context.renderFinished, nullptr);
    }
    if (context.inFlight != VK_NULL_HANDLE) {
        vkDestroyFence(m_device, context.inFlight, nullptr);
    }
    context.imageAvailable = VK_NULL_HANDLE;
    context.renderFinished = VK_NULL_HANDLE;
    context.inFlight = VK_NULL_HANDLE;
    context.commandBuffers.clear();
}

bool VulkanRendering::CreateSwapchainResources(WindowContext& context, unsigned int width, unsigned int height)
{
    NeoWPF::Diagnostics::LogInfo("[Vulkan] Swapchain setup begin: requestedExtent=%ux%u", width, height);
    VkSurfaceCapabilitiesKHR capabilities{};
    const VkResult capabilitiesResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        m_physicalDevice, context.surface, &capabilities);
    if (capabilitiesResult != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Surface capabilities query failed: %d",
            static_cast<int>(capabilitiesResult));
        return false;
    }
    NeoWPF::Diagnostics::LogInfo("[Vulkan] Surface capabilities: minImages=%u maxImages=%u currentExtent=%ux%u",
        capabilities.minImageCount, capabilities.maxImageCount,
        capabilities.currentExtent.width, capabilities.currentExtent.height);

    std::uint32_t formatCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, context.surface, &formatCount, nullptr);
    if (result != VK_SUCCESS || formatCount == 0) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Surface format count query failed: result=%d count=%u",
            static_cast<int>(result), formatCount);
        return false;
    }
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, context.surface, &formatCount, formats.data());
    if (result != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Surface format list query failed: %d", static_cast<int>(result));
        return false;
    }
    const auto formatIterator = std::find_if(formats.begin(), formats.end(), [](const auto& candidate) {
        return candidate.format == VK_FORMAT_B8G8R8A8_UNORM ||
               candidate.format == VK_FORMAT_R8G8B8A8_UNORM;
    });
    const auto format = formatIterator != formats.end() ? *formatIterator : formats[0];
    context.swapchainFormat = format.format;
    context.swapchainExtent = capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()
        ? capabilities.currentExtent
        : VkExtent2D{
            std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
    if (context.swapchainExtent.width == 0 || context.swapchainExtent.height == 0) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Surface selected a zero-sized swapchain extent");
        return false;
    }
    NeoWPF::Diagnostics::LogInfo("[Vulkan] Swapchain format=%d colorSpace=%d extent=%ux%u",
        static_cast<int>(format.format), static_cast<int>(format.colorSpace),
        context.swapchainExtent.width, context.swapchainExtent.height);

    const auto compositeAlpha = SelectSurfaceCompositeAlpha(
        context.transparent, capabilities.supportedCompositeAlpha);
    if (!compositeAlpha) {
        NeoWPF::Diagnostics::LogError(
            "[Vulkan] Surface does not support the required composite alpha mode for this window");
        return false;
    }
    DestroySwapchainResources(context);
    const std::uint32_t requestedImageCount = (std::max)(capabilities.minImageCount, 2u);
    const std::uint32_t imageCount = std::min(
        requestedImageCount,
        capabilities.maxImageCount == 0 ? requestedImageCount : capabilities.maxImageCount);
    const std::array<std::uint32_t, 2> queueFamilies{
        m_graphicsQueueFamily, m_presentQueueFamily};
    const bool separateQueues = m_graphicsQueueFamily != m_presentQueueFamily;
    const VkSwapchainCreateInfoKHR swapchainInfo{
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr, 0, context.surface, imageCount,
        format.format, format.colorSpace, context.swapchainExtent, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        separateQueues ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
        separateQueues ? 2u : 0u, separateQueues ? queueFamilies.data() : nullptr,
        capabilities.currentTransform, *compositeAlpha, VK_PRESENT_MODE_FIFO_KHR,
        VK_TRUE, VK_NULL_HANDLE};
    result = vkCreateSwapchainKHR(m_device, &swapchainInfo, nullptr, &context.swapchain);
    if (result != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] vkCreateSwapchainKHR failed: %d", static_cast<int>(result));
        return false;
    }
    NeoWPF::Diagnostics::LogInfo("[Vulkan] Swapchain created: requestedImages=%u", imageCount);

    std::uint32_t actualCount = 0;
    result = vkGetSwapchainImagesKHR(m_device, context.swapchain, &actualCount, nullptr);
    if (result != VK_SUCCESS || actualCount == 0) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Swapchain image count query failed: result=%d count=%u",
            static_cast<int>(result), actualCount);
        DestroySwapchainResources(context);
        return false;
    }
    context.swapchainImages.resize(actualCount);
    result = vkGetSwapchainImagesKHR(m_device, context.swapchain, &actualCount, context.swapchainImages.data());
    if (result != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Swapchain image list query failed: %d", static_cast<int>(result));
        DestroySwapchainResources(context);
        return false;
    }
    context.swapchainImageViews.resize(actualCount);
    for (std::uint32_t index = 0; index < actualCount; ++index) {
        const VkImageViewCreateInfo viewInfo{
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0,
            context.swapchainImages[index], VK_IMAGE_VIEW_TYPE_2D, context.swapchainFormat,
            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
             VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
        if (vkCreateImageView(m_device, &viewInfo, nullptr, &context.swapchainImageViews[index]) != VK_SUCCESS) {
            DestroySwapchainResources(context);
            return false;
        }
    }

    const VkAttachmentDescription colorAttachment{
        0, context.swapchainFormat, VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
    const VkAttachmentReference colorReference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    const VkSubpassDescription subpass{
        0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &colorReference,
        nullptr, nullptr, 0, nullptr};
    const VkSubpassDependency dependency{
        VK_SUBPASS_EXTERNAL, 0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0};
    const VkRenderPassCreateInfo renderPassInfo{
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0,
        1, &colorAttachment, 1, &subpass, 1, &dependency};
    if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &context.renderPass) != VK_SUCCESS) {
        DestroySwapchainResources(context);
        return false;
    }
    if (!CreateSolidPipeline(context.renderPass)) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Solid pipeline creation failed");
        DestroySwapchainResources(context);
        return false;
    }
    if (m_textureDescriptorPool != VK_NULL_HANDLE && !CreateTexturedPipeline(context.renderPass)) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Textured pipeline recreation failed");
        DestroySwapchainResources(context);
        return false;
    }
    context.framebuffers.resize(actualCount);
    for (std::uint32_t index = 0; index < actualCount; ++index) {
        const VkFramebufferCreateInfo framebufferInfo{
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr,
            0, context.renderPass, 1, &context.swapchainImageViews[index],
            context.swapchainExtent.width, context.swapchainExtent.height, 1};
        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &context.framebuffers[index]) != VK_SUCCESS) {
            DestroySwapchainResources(context);
            return false;
        }
    }

    const VkCommandPoolCreateInfo commandPoolInfo{
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, m_graphicsQueueFamily};
    if (vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &context.commandPool) != VK_SUCCESS) {
        DestroySwapchainResources(context);
        return false;
    }
    context.commandBuffers.resize(actualCount);
    const VkCommandBufferAllocateInfo commandBufferInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
        context.commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, actualCount};
    if (vkAllocateCommandBuffers(m_device, &commandBufferInfo, context.commandBuffers.data()) != VK_SUCCESS) {
        DestroySwapchainResources(context);
        return false;
    }

    const VkSemaphoreCreateInfo semaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
    const VkFenceCreateInfo fenceInfo{
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};
    if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &context.imageAvailable) != VK_SUCCESS ||
        vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &context.renderFinished) != VK_SUCCESS ||
        vkCreateFence(m_device, &fenceInfo, nullptr, &context.inFlight) != VK_SUCCESS) {
        DestroySwapchainResources(context);
        return false;
    }
    return true;
}

}
