#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Vulkan/VulkanRendering.h>

#include "textured.frag.spv.h"
#include "textured.vert.spv.h"

#include <cstring>
#include <functional>
#include <limits>
#include <vector>

namespace NeoWPF::Rendering::Vulkan {
namespace {

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

VkShaderModule CreateShaderModule(VkDevice device, const std::uint32_t* code, std::size_t size)
{
    const VkShaderModuleCreateInfo info{
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, size, code};
    VkShaderModule module = VK_NULL_HANDLE;
    return vkCreateShaderModule(device, &info, nullptr, &module) == VK_SUCCESS
        ? module : VK_NULL_HANDLE;
}

bool CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size,
                  VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                  VkBuffer& buffer, VkDeviceMemory& memory)
{
    const VkBufferCreateInfo bufferInfo{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, size, usage,
        VK_SHARING_MODE_EXCLUSIVE, 0, nullptr};
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        return false;
    }
    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(device, buffer, &requirements);
    const std::uint32_t memoryType = FindMemoryType(
        physicalDevice, requirements.memoryTypeBits, properties);
    if (memoryType == std::numeric_limits<std::uint32_t>::max()) {
        vkDestroyBuffer(device, buffer, nullptr);
        buffer = VK_NULL_HANDLE;
        return false;
    }
    const VkMemoryAllocateInfo allocationInfo{
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, requirements.size, memoryType};
    if (vkAllocateMemory(device, &allocationInfo, nullptr, &memory) != VK_SUCCESS ||
        vkBindBufferMemory(device, buffer, memory, 0) != VK_SUCCESS) {
        if (memory != VK_NULL_HANDLE) vkFreeMemory(device, memory, nullptr);
        vkDestroyBuffer(device, buffer, nullptr);
        buffer = VK_NULL_HANDLE;
        memory = VK_NULL_HANDLE;
        return false;
    }
    return true;
}

bool SubmitImmediate(VkDevice device, VkCommandPool commandPool, VkQueue queue,
                     const std::function<void(VkCommandBuffer)>& record)
{
    const VkCommandBufferAllocateInfo allocationInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(device, &allocationInfo, &commandBuffer) != VK_SUCCESS) {
        return false;
    }
    const VkCommandBufferBeginInfo beginInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        return false;
    }
    record(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        return false;
    }
    const VkSubmitInfo submitInfo{
        VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1,
        &commandBuffer, 0, nullptr};
    const VkResult submitResult = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    const VkResult waitResult = submitResult == VK_SUCCESS ? vkQueueWaitIdle(queue) : submitResult;
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    return waitResult == VK_SUCCESS;
}

}

bool VulkanRendering::EnsureVertexBuffer(VertexBuffer& buffer, VkDeviceSize requiredSize)
{
    if (requiredSize <= buffer.capacity && buffer.buffer != VK_NULL_HANDLE &&
        buffer.memory != VK_NULL_HANDLE) {
        return true;
    }
    if (m_device == VK_NULL_HANDLE) {
        return false;
    }

    VkDeviceSize newCapacity = buffer.capacity > 0 ? buffer.capacity : 1024 * 1024;
    while (newCapacity < requiredSize) {
        if (newCapacity > std::numeric_limits<VkDeviceSize>::max() / 2) {
            newCapacity = requiredSize;
            break;
        }
        newCapacity *= 2;
    }

    VkBuffer newBuffer = VK_NULL_HANDLE;
    VkDeviceMemory newMemory = VK_NULL_HANDLE;
    if (!CreateBuffer(m_physicalDevice, m_device, newCapacity,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      newBuffer, newMemory)) {
        return false;
    }
    if (buffer.buffer != VK_NULL_HANDLE) vkDestroyBuffer(m_device, buffer.buffer, nullptr);
    if (buffer.memory != VK_NULL_HANDLE) vkFreeMemory(m_device, buffer.memory, nullptr);
    buffer.buffer = newBuffer;
    buffer.memory = newMemory;
    buffer.capacity = newCapacity;
    return true;
}

bool VulkanRendering::CreateTexturedPipeline(WindowContext& context, VkRenderPass renderPass)
{
    if (renderPass == VK_NULL_HANDLE) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Cannot create textured pipeline without a render pass");
        return false;
    }
    if (m_textureDescriptorPool == VK_NULL_HANDLE) {
        const VkDescriptorSetLayoutBinding binding{
            0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
            VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        const VkDescriptorSetLayoutCreateInfo setLayoutInfo{
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, 1, &binding};
        if (vkCreateDescriptorSetLayout(m_device, &setLayoutInfo, nullptr, &m_textureSetLayout) != VK_SUCCESS) {
            return false;
        }
        const VkDescriptorPoolSize poolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024};
        const VkDescriptorPoolCreateInfo poolInfo{
            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0, 1024, 1, &poolSize};
        if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_textureDescriptorPool) != VK_SUCCESS) {
            return false;
        }
        const VkSamplerCreateInfo samplerInfo{
            VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0,
            VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 0.0f, VK_FALSE, 1.0f, VK_FALSE,
            VK_COMPARE_OP_ALWAYS, 0.0f, 0.0f, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE};
        if (vkCreateSampler(m_device, &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
            return false;
        }
    }
    const VkPushConstantRange pushConstants{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 2};
    const VkPipelineLayoutCreateInfo layoutInfo{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 1, &m_textureSetLayout,
        1, &pushConstants};
    if (vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &context.texturedPipelineLayout) != VK_SUCCESS) {
        return false;
    }
    const VkShaderModule vertexShader = CreateShaderModule(
        m_device, neowpf_textured_vert_spv, sizeof(neowpf_textured_vert_spv));
    const VkShaderModule fragmentShader = CreateShaderModule(
        m_device, neowpf_textured_frag_spv, sizeof(neowpf_textured_frag_spv));
    if (vertexShader == VK_NULL_HANDLE || fragmentShader == VK_NULL_HANDLE) {
        return false;
    }
    const VkPipelineShaderStageCreateInfo stages[] = {
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
         VK_SHADER_STAGE_VERTEX_BIT, vertexShader, "main", nullptr},
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
         VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader, "main", nullptr}};
    const VkVertexInputBindingDescription vertexBinding{0, sizeof(float) * 8, VK_VERTEX_INPUT_RATE_VERTEX};
    const VkVertexInputAttributeDescription attributes[] = {
        {0, 0, VK_FORMAT_R32G32_SFLOAT, 0},
        {1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 2},
        {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4}};
    const VkPipelineVertexInputStateCreateInfo vertexInput{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0,
        1, &vertexBinding, 3, attributes};
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
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0, 2, stages,
        &vertexInput, &inputAssembly, nullptr, &viewportState, &rasterization, &multisample,
        nullptr, &blend, &dynamicState, context.texturedPipelineLayout, renderPass, 0, VK_NULL_HANDLE, -1};
    const VkResult pipelineResult = vkCreateGraphicsPipelines(
        m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &context.texturedPipeline);
    vkDestroyShaderModule(m_device, vertexShader, nullptr);
    vkDestroyShaderModule(m_device, fragmentShader, nullptr);
    if (pipelineResult != VK_SUCCESS) {
        return false;
    }
    return true;
}

void VulkanRendering::DestroyTexturedPipeline(WindowContext& context) noexcept
{
    if (m_device == VK_NULL_HANDLE) {
        return;
    }
    if (context.texturedPipeline != VK_NULL_HANDLE) vkDestroyPipeline(m_device, context.texturedPipeline, nullptr);
    if (context.texturedPipelineLayout != VK_NULL_HANDLE) vkDestroyPipelineLayout(m_device, context.texturedPipelineLayout, nullptr);
    context.texturedPipeline = VK_NULL_HANDLE;
    context.texturedPipelineLayout = VK_NULL_HANDLE;
}

void VulkanRendering::DestroyTextureResources() noexcept
{
    if (m_device == VK_NULL_HANDLE) {
        return;
    }
    ClearTextures();
    if (m_textureSampler != VK_NULL_HANDLE) vkDestroySampler(m_device, m_textureSampler, nullptr);
    if (m_textureDescriptorPool != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_device, m_textureDescriptorPool, nullptr);
    if (m_textureSetLayout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(m_device, m_textureSetLayout, nullptr);
    m_textureSampler = VK_NULL_HANDLE;
    m_textureDescriptorPool = VK_NULL_HANDLE;
    m_textureSetLayout = VK_NULL_HANDLE;
}

std::uint32_t VulkanRendering::CreateTexture(unsigned int width, unsigned int height, const void* rgbaData)
{
    std::lock_guard<std::recursive_mutex> renderLock(m_renderMutex);
    WindowContext* context = nullptr;
    {
        std::lock_guard<std::mutex> contextLock(m_windowContextsMutex);
        for (const auto& [handle, window] : m_windowContexts) {
            (void)handle;
            if (window->renderPass != VK_NULL_HANDLE && window->commandPool != VK_NULL_HANDLE) {
                context = window.get();
                break;
            }
        }
    }
    NeoWPF::Diagnostics::Log(
        NeoWPF::Diagnostics::Category::TextRendering,
        NeoWPF::Diagnostics::Level::Debug,
        "[Vulkan] CreateTexture request size=%ux%u data=%p initialized=%d context=%p commandPool=%p",
        width, height, rgbaData, m_initialized ? 1 : 0, context,
        context ? static_cast<void*>(context->commandPool) : nullptr);
    if (!m_initialized || !context || context->commandPool == VK_NULL_HANDLE ||
        width == 0 || height == 0 || rgbaData == nullptr) {
        return 0;
    }
    if (m_textureDescriptorPool == VK_NULL_HANDLE && !CreateTexturedPipeline(*context, context->renderPass)) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Failed to initialize textured resources on demand");
        return 0;
    }
    {
        std::lock_guard<std::mutex> contextLock(m_windowContextsMutex);
        for (const auto& [handle, window] : m_windowContexts) {
            (void)handle;
            if (window.get() != context && window->renderPass != VK_NULL_HANDLE &&
                window->commandPool != VK_NULL_HANDLE && window->texturedPipeline == VK_NULL_HANDLE &&
                !CreateTexturedPipeline(*window, window->renderPass)) {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
                    NeoWPF::Diagnostics::Level::Debug,
                    "[Vulkan] Failed to create the textured pipeline for an open window");
                return 0;
            }
        }
    }
    const VkDeviceSize dataSize = static_cast<VkDeviceSize>(width) * height * 4;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    if (!CreateBuffer(m_physicalDevice, m_device, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      stagingBuffer, stagingMemory)) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Failed to create texture staging buffer");
        return 0;
    }
    void* mapped = nullptr;
    if (vkMapMemory(m_device, stagingMemory, 0, dataSize, 0, &mapped) != VK_SUCCESS) {
        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        vkFreeMemory(m_device, stagingMemory, nullptr);
        return 0;
    }
    std::memcpy(mapped, rgbaData, static_cast<std::size_t>(dataSize));
    vkUnmapMemory(m_device, stagingMemory);

    TextureResource texture;
    const VkImageCreateInfo imageInfo{
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0, VK_IMAGE_TYPE_2D,
        VK_FORMAT_R8G8B8A8_UNORM, {width, height, 1}, 1, 1, VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED};
    if (vkCreateImage(m_device, &imageInfo, nullptr, &texture.image) != VK_SUCCESS) {
        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        vkFreeMemory(m_device, stagingMemory, nullptr);
        return 0;
    }
    VkMemoryRequirements imageRequirements{};
    vkGetImageMemoryRequirements(m_device, texture.image, &imageRequirements);
    const std::uint32_t memoryType = FindMemoryType(
        m_physicalDevice, imageRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    const VkMemoryAllocateInfo imageAllocation{
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, imageRequirements.size, memoryType};
    if (memoryType == std::numeric_limits<std::uint32_t>::max() ||
        vkAllocateMemory(m_device, &imageAllocation, nullptr, &texture.memory) != VK_SUCCESS ||
        vkBindImageMemory(m_device, texture.image, texture.memory, 0) != VK_SUCCESS) {
        vkDestroyImage(m_device, texture.image, nullptr);
        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        vkFreeMemory(m_device, stagingMemory, nullptr);
        return 0;
    }
    const bool uploaded = SubmitImmediate(m_device, context->commandPool, m_graphicsQueue, [&](VkCommandBuffer commandBuffer) {
        const VkImageMemoryBarrier toTransfer{
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED, texture.image, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toTransfer);
        const VkBufferImageCopy copy{
            0, 0, 0, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, {0, 0, 0}, {width, height, 1}};
        vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, texture.image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
        const VkImageMemoryBarrier toShader{
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED, texture.image, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toShader);
    });
    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingMemory, nullptr);
    if (!uploaded) {
        vkDestroyImage(m_device, texture.image, nullptr);
        vkFreeMemory(m_device, texture.memory, nullptr);
        return 0;
    }
    NeoWPF::Diagnostics::Log(
        NeoWPF::Diagnostics::Category::TextRendering,
        NeoWPF::Diagnostics::Level::Debug,
        "[Vulkan] Texture upload complete image=%p memory=%p size=%ux%u",
        static_cast<void*>(texture.image), static_cast<void*>(texture.memory), width, height);
    const VkImageViewCreateInfo viewInfo{
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0, texture.image,
        VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM,
        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
    if (vkCreateImageView(m_device, &viewInfo, nullptr, &texture.view) != VK_SUCCESS) {
        vkDestroyImage(m_device, texture.image, nullptr);
        vkFreeMemory(m_device, texture.memory, nullptr);
        return 0;
    }
    const VkDescriptorSetAllocateInfo descriptorInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
        m_textureDescriptorPool, 1, &m_textureSetLayout};
    if (vkAllocateDescriptorSets(m_device, &descriptorInfo, &texture.descriptor) != VK_SUCCESS) {
        vkDestroyImageView(m_device, texture.view, nullptr);
        vkDestroyImage(m_device, texture.image, nullptr);
        vkFreeMemory(m_device, texture.memory, nullptr);
        return 0;
    }
    const VkDescriptorImageInfo imageDescriptor{
        m_textureSampler, texture.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    const VkWriteDescriptorSet write{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, texture.descriptor, 0, 0, 1,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &imageDescriptor, nullptr, nullptr};
    vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
    std::lock_guard<std::mutex> lock(m_textureMutex);
    std::uint32_t id = m_nextTextureId++;
    if (id == 0) {
        id = m_nextTextureId++;
    }
    m_textures.emplace(id, texture);
    NeoWPF::Diagnostics::Log(
        NeoWPF::Diagnostics::Category::TextRendering,
        NeoWPF::Diagnostics::Level::Debug,
        "[Vulkan] Texture created id=%u image=%p view=%p descriptor=%p",
        id, static_cast<void*>(texture.image), static_cast<void*>(texture.view),
        static_cast<void*>(texture.descriptor));
    return id;
}

void VulkanRendering::DestroyTexture(std::uint32_t id) noexcept
{
    std::lock_guard<std::recursive_mutex> renderLock(m_renderMutex);
    std::lock_guard<std::mutex> lock(m_textureMutex);
    const auto iterator = m_textures.find(id);
    if (iterator == m_textures.end()) return;
    if (iterator->second.view != VK_NULL_HANDLE) vkDestroyImageView(m_device, iterator->second.view, nullptr);
    if (iterator->second.image != VK_NULL_HANDLE) vkDestroyImage(m_device, iterator->second.image, nullptr);
    if (iterator->second.memory != VK_NULL_HANDLE) vkFreeMemory(m_device, iterator->second.memory, nullptr);
    m_textures.erase(iterator);
}

void VulkanRendering::ClearTextures() noexcept
{
    std::lock_guard<std::recursive_mutex> renderLock(m_renderMutex);
    std::lock_guard<std::mutex> lock(m_textureMutex);
    for (const auto& [id, texture] : m_textures) {
        if (texture.view != VK_NULL_HANDLE) vkDestroyImageView(m_device, texture.view, nullptr);
        if (texture.image != VK_NULL_HANDLE) vkDestroyImage(m_device, texture.image, nullptr);
        if (texture.memory != VK_NULL_HANDLE) vkFreeMemory(m_device, texture.memory, nullptr);
    }
    m_textures.clear();
    m_nextTextureId = 1;
}

}
