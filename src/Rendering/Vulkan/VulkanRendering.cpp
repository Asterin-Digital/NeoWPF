#include <NeoWPF/Rendering/Vulkan/VulkanRendering.h>

#include <NeoWPF/Rendering/Renderer.h>

namespace NeoWPF::Rendering::Vulkan {

bool VulkanRendering::Initialize(void*)
{
    if (m_initialized) {
        return true;
    }

    const VkApplicationInfo applicationInfo{
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        nullptr,
        "NeoWPF",
        VK_MAKE_VERSION(1, 0, 0),
        "NeoWPF Vulkan backend",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_0,
    };
    const VkInstanceCreateInfo instanceCreateInfo{
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        nullptr,
        0,
        &applicationInfo,
        0,
        nullptr,
        0,
        nullptr,
    };

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance) != VK_SUCCESS) {
        return false;
    }

    m_initialized = true;
    return true;
}

void VulkanRendering::Shutdown() noexcept
{
    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
    m_initialized = false;
}

bool VulkanRendering::IsInitialized() const noexcept
{
    return m_initialized;
}

void VulkanRendering::Resize(unsigned int, unsigned int) noexcept
{
}

void VulkanRendering::BeginFrame()
{
}

void VulkanRendering::Execute(const NeoWPF::Rendering::CommandList&)
{
}

void VulkanRendering::Present()
{
}

std::uint32_t VulkanRendering::CreateTexture(unsigned int, unsigned int, const void*)
{
    return 0;
}

void VulkanRendering::DestroyTexture(std::uint32_t) noexcept
{
}

void VulkanRendering::ClearTextures() noexcept
{
}

}
