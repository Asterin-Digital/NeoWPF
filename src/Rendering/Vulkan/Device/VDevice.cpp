#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Vulkan/VulkanRendering.h>

#include "solid.frag.spv.h"
#include "solid.vert.spv.h"

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

bool HasDeviceExtension(VkPhysicalDevice device, const char* name)
{
    std::uint32_t extensionCount = 0;
    if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
        return false;
    }
    std::vector<VkExtensionProperties> extensions(extensionCount);
    if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data()) != VK_SUCCESS) {
        return false;
    }
    return HasExtension(extensions, name);
}

std::uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, std::uint32_t typeFilter,
                             VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    for (std::uint32_t index = 0; index < memoryProperties.memoryTypeCount; ++index) {
        if ((typeFilter & (1u << index)) &&
            (memoryProperties.memoryTypes[index].propertyFlags & properties) == properties) {
            return index;
        }
    }
    return std::numeric_limits<std::uint32_t>::max();
}

VkShaderModule CreateShaderModule(VkDevice device, const std::uint32_t* code, std::size_t codeSize)
{
    const VkShaderModuleCreateInfo createInfo{
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, codeSize, code};
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    return vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) == VK_SUCCESS
        ? shaderModule : VK_NULL_HANDLE;
}

}

bool VulkanRendering::InitializeDevice(void* nativeWindowHandle)
{
    auto* windowContext = FindWindowContext(nativeWindowHandle);
    if (!windowContext) {
        return false;
    }
    std::uint32_t deviceCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    if (result != VK_SUCCESS || deviceCount == 0) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Physical device count query failed: %s (%d), count=%u",
            VkResultName(result), static_cast<int>(result), deviceCount);
        return false;
    }
    NeoWPF::Diagnostics::LogInfo("[Vulkan] Physical devices found: %u", deviceCount);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    result = vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    if (result != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Physical device list query failed: %s (%d)",
            VkResultName(result), static_cast<int>(result));
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
        if (!HasDeviceExtension(device, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
            NeoWPF::Diagnostics::LogWarn("[Vulkan] Device[%u] rejected: missing %s",
                deviceIndex, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
            continue;
        }
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
                device, index, windowContext->surface, &presentSupported);
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
    result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
    if (result != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] vkCreateDevice failed: %s (%d)",
            VkResultName(result), static_cast<int>(result));
        return false;
    }
    NeoWPF::Diagnostics::LogInfo("[Vulkan] Logical device created: %p", static_cast<void*>(m_device));
    vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_presentQueueFamily, 0, &m_presentQueue);
    return true;
}

void VulkanRendering::DestroySolidPipeline() noexcept
{
    if (m_device == VK_NULL_HANDLE) {
        return;
    }
    if (m_solidPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device, m_solidPipeline, nullptr);
        m_solidPipeline = VK_NULL_HANDLE;
    }
    if (m_solidPipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_device, m_solidPipelineLayout, nullptr);
        m_solidPipelineLayout = VK_NULL_HANDLE;
    }
    if (m_solidVertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device, m_solidVertexBuffer, nullptr);
        m_solidVertexBuffer = VK_NULL_HANDLE;
    }
    if (m_solidVertexMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, m_solidVertexMemory, nullptr);
        m_solidVertexMemory = VK_NULL_HANDLE;
    }
}

bool VulkanRendering::CreateSolidPipeline()
{
    auto* context = FirstWindowContext();
    if (!context) {
        return false;
    }
    const VkPushConstantRange pushConstants{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 2};
    const VkPipelineLayoutCreateInfo layoutInfo{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &pushConstants};
    if (vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &m_solidPipelineLayout) != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] vkCreatePipelineLayout failed for solid pipeline");
        return false;
    }
    const VkShaderModule vertexShader = CreateShaderModule(
        m_device, neowpf_solid_vert_spv, sizeof(neowpf_solid_vert_spv));
    const VkShaderModule fragmentShader = CreateShaderModule(
        m_device, neowpf_solid_frag_spv, sizeof(neowpf_solid_frag_spv));
    if (vertexShader == VK_NULL_HANDLE || fragmentShader == VK_NULL_HANDLE) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Failed to create solid shader module");
        if (vertexShader != VK_NULL_HANDLE) vkDestroyShaderModule(m_device, vertexShader, nullptr);
        if (fragmentShader != VK_NULL_HANDLE) vkDestroyShaderModule(m_device, fragmentShader, nullptr);
        return false;
    }
    const VkPipelineShaderStageCreateInfo shaderStages[] = {
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
         VK_SHADER_STAGE_VERTEX_BIT, vertexShader, "main", nullptr},
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
         VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader, "main", nullptr}};
    const VkVertexInputBindingDescription binding{0, sizeof(float) * 6, VK_VERTEX_INPUT_RATE_VERTEX};
    const VkVertexInputAttributeDescription attributes[] = {
        {0, 0, VK_FORMAT_R32G32_SFLOAT, 0},
        {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 2}};
    const VkPipelineVertexInputStateCreateInfo vertexInput{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0,
        1, &binding, 2, attributes};
    const VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE};
    const VkViewport viewport{0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
    const VkRect2D scissor{{0, 0}, {1, 1}};
    const VkPipelineViewportStateCreateInfo viewportState{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, 0, 1, &viewport, 1, &scissor};
    const VkPipelineRasterizationStateCreateInfo rasterization{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_FALSE,
        VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE,
        0.0f, 0.0f, 0.0f, 1.0f};
    const VkPipelineMultisampleStateCreateInfo multisample{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr, 0,
        VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE};
    const VkPipelineColorBlendAttachmentState blendAttachment{
        VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
    const VkPipelineColorBlendStateCreateInfo blend{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0, VK_FALSE,
        VK_LOGIC_OP_COPY, 1, &blendAttachment, {1.0f, 1.0f, 1.0f, 1.0f}};
    const VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    const VkPipelineDynamicStateCreateInfo dynamicState{
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, 2, dynamicStates};
    const VkGraphicsPipelineCreateInfo pipelineInfo{
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0, 2, shaderStages,
        &vertexInput, &inputAssembly, nullptr, &viewportState, &rasterization, &multisample,
        nullptr, &blend, &dynamicState, m_solidPipelineLayout, context->renderPass, 0, VK_NULL_HANDLE, -1};
    const VkResult pipelineResult = vkCreateGraphicsPipelines(
        m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_solidPipeline);
    vkDestroyShaderModule(m_device, vertexShader, nullptr);
    vkDestroyShaderModule(m_device, fragmentShader, nullptr);
    if (pipelineResult != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] vkCreateGraphicsPipelines failed: %d",
            static_cast<int>(pipelineResult));
        return false;
    }

    const VkBufferCreateInfo bufferInfo{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, 1024 * 1024,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr};
    if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_solidVertexBuffer) != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] vkCreateBuffer failed for solid vertices");
        return false;
    }
    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(m_device, m_solidVertexBuffer, &memoryRequirements);
    const std::uint32_t memoryType = FindMemoryType(
        m_physicalDevice, memoryRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (memoryType == std::numeric_limits<std::uint32_t>::max()) {
        NeoWPF::Diagnostics::LogError("[Vulkan] No host-visible memory type for solid vertices");
        return false;
    }
    const VkMemoryAllocateInfo allocationInfo{
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, memoryRequirements.size, memoryType};
    if (vkAllocateMemory(m_device, &allocationInfo, nullptr, &m_solidVertexMemory) != VK_SUCCESS ||
        vkBindBufferMemory(m_device, m_solidVertexBuffer, m_solidVertexMemory, 0) != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Solid vertex buffer memory setup failed");
        return false;
    }
    return true;
}

}
