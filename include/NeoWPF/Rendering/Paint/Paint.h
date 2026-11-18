/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>

namespace NeoWPF::Rendering {

struct Paint {
    enum class Kind : std::uint8_t {
        Solid,
        LinearGradient,
        RadialGradient,
    };

    Kind kind = Kind::Solid;
    std::uint32_t color = 0;
    std::uint32_t gradientIndex = 0;

    static Paint Solid(std::uint32_t argb) noexcept
    {
        return {Kind::Solid, argb, 0};
    }

    static Paint LinearGradient(std::uint32_t index) noexcept
    {
        return {Kind::LinearGradient, 0, index};
    }

    static Paint RadialGradient(std::uint32_t index) noexcept
    {
        return {Kind::RadialGradient, 0, index};
    }

    bool IsEmpty() const noexcept
    {
        return kind == Kind::Solid && ((color >> 24) & 0xFFu) == 0;
    }
};

}
