#include <NeoWPF/Rendering/Vulkan/VulkanRendering.h>

#include <algorithm>
#include <array>
#include <limits>
#include <vector>

namespace NeoWPF::Rendering::Vulkan {

void VulkanRendering::DestroySwapchainResources() noexcept
{
    if (m_device == VK_NULL_HANDLE) {
        return;
    }
    for (auto framebuffer : m_framebuffers) {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
    m_framebuffers.clear();
    if (m_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    }
    m_renderPass = VK_NULL_HANDLE;
    for (auto view : m_swapchainImageViews) {
        vkDestroyImageView(m_device, view, nullptr);
    }
    m_swapchainImageViews.clear();
    if (m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    }
    m_swapchain = VK_NULL_HANDLE;
    if (m_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    }
    m_commandPool = VK_NULL_HANDLE;
    if (m_imageAvailable != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, m_imageAvailable, nullptr);
    }
    if (m_renderFinished != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, m_renderFinished, nullptr);
    }
    if (m_inFlight != VK_NULL_HANDLE) {
        vkDestroyFence(m_device, m_inFlight, nullptr);
    }
    m_imageAvailable = VK_NULL_HANDLE;
    m_renderFinished = VK_NULL_HANDLE;
    m_inFlight = VK_NULL_HANDLE;
    m_commandBuffers.clear();
}

bool VulkanRendering::CreateSwapchainResources(unsigned int width, unsigned int height)
{
    VkSurfaceCapabilitiesKHR capabilities{};
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities) != VK_SUCCESS) {
        return false;
    }

    std::uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
    if (formatCount == 0) {
        return false;
    }
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data());
    const auto format = formats[0];
    m_swapchainFormat = format.format;
    m_swapchainExtent = capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()
        ? capabilities.currentExtent
        : VkExtent2D{
            std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
    if (m_swapchainExtent.width == 0 || m_swapchainExtent.height == 0) {
        return false;
    }

    DestroySwapchainResources();
    const std::uint32_t requestedImageCount = (std::max)(capabilities.minImageCount, 2u);
    const std::uint32_t imageCount = std::min(
        requestedImageCount,
        capabilities.maxImageCount == 0 ? requestedImageCount : capabilities.maxImageCount);
    const std::array<std::uint32_t, 2> queueFamilies{
        m_graphicsQueueFamily, m_presentQueueFamily};
    const bool separateQueues = m_graphicsQueueFamily != m_presentQueueFamily;
    const VkSwapchainCreateInfoKHR swapchainInfo{
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr, 0, m_surface, imageCount,
        format.format, format.colorSpace, m_swapchainExtent, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        separateQueues ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
        separateQueues ? 2u : 0u, separateQueues ? queueFamilies.data() : nullptr,
        capabilities.currentTransform, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_PRESENT_MODE_FIFO_KHR,
        VK_TRUE, VK_NULL_HANDLE};
    if (vkCreateSwapchainKHR(m_device, &swapchainInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        return false;
    }

    std::uint32_t actualCount = 0;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualCount, nullptr);
    m_swapchainImages.resize(actualCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualCount, m_swapchainImages.data());
    m_swapchainImageViews.resize(actualCount);
    for (std::uint32_t index = 0; index < actualCount; ++index) {
        const VkImageViewCreateInfo viewInfo{
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0,
            m_swapchainImages[index], VK_IMAGE_VIEW_TYPE_2D, m_swapchainFormat,
            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
             VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
        if (vkCreateImageView(m_device, &viewInfo, nullptr, &m_swapchainImageViews[index]) != VK_SUCCESS) {
            DestroySwapchainResources();
            return false;
        }
    }

    const VkAttachmentDescription colorAttachment{
        0, m_swapchainFormat, VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
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
    if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        DestroySwapchainResources();
        return false;
    }

    m_framebuffers.resize(actualCount);
    for (std::uint32_t index = 0; index < actualCount; ++index) {
        const VkFramebufferCreateInfo framebufferInfo{
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr,
            0, m_renderPass, 1, &m_swapchainImageViews[index],
            m_swapchainExtent.width, m_swapchainExtent.height, 1};
        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_framebuffers[index]) != VK_SUCCESS) {
            DestroySwapchainResources();
            return false;
        }
    }

    const VkCommandPoolCreateInfo commandPoolInfo{
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, m_graphicsQueueFamily};
    if (vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        DestroySwapchainResources();
        return false;
    }
    m_commandBuffers.resize(actualCount);
    const VkCommandBufferAllocateInfo commandBufferInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
        m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, actualCount};
    if (vkAllocateCommandBuffers(m_device, &commandBufferInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        DestroySwapchainResources();
        return false;
    }

    const VkSemaphoreCreateInfo semaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
    const VkFenceCreateInfo fenceInfo{
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};
    if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailable) != VK_SUCCESS ||
        vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinished) != VK_SUCCESS ||
        vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlight) != VK_SUCCESS) {
        DestroySwapchainResources();
        return false;
    }
    return true;
}

}
