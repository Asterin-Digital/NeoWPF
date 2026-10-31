/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>
#include <optional>

namespace NeoWPF::Rendering::Vulkan {

inline std::optional<VkCompositeAlphaFlagBitsKHR> SelectSurfaceCompositeAlpha(
    bool transparent, VkCompositeAlphaFlagsKHR supported) noexcept
{
    if (transparent) {
        if ((supported & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) != 0) {
            return VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
        }
        return std::nullopt;
    }
    if ((supported & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) != 0) {
        return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }
    if ((supported & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) != 0) {
        return VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }
    return std::nullopt;
}

inline std::array<float, 4> ToSurfaceClearColor(std::uint32_t argb,
                                               bool premultiplied) noexcept
{
    const float alpha = static_cast<float>((argb >> 24) & 0xFF) / 255.0f;
    const float red = static_cast<float>((argb >> 16) & 0xFF) / 255.0f;
    const float green = static_cast<float>((argb >> 8) & 0xFF) / 255.0f;
    const float blue = static_cast<float>(argb & 0xFF) / 255.0f;
    const float multiplier = premultiplied ? alpha : 1.0f;
    return {red * multiplier, green * multiplier, blue * multiplier, alpha};
}

}