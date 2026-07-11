#pragma once

#include <cstdint>

namespace NeoWPF::UI {

struct Color {
    std::uint8_t A = 0xFF;
    std::uint8_t R = 0x00;
    std::uint8_t G = 0x00;
    std::uint8_t B = 0x00;

    constexpr Color() noexcept = default;

    constexpr Color(std::uint8_t a, std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept
        : A(a), R(r), G(g), B(b) {}

    constexpr Color(std::uint32_t argb) noexcept
        : A(static_cast<std::uint8_t>((argb >> 24) & 0xFFu)),
          R(static_cast<std::uint8_t>((argb >> 16) & 0xFFu)),
          G(static_cast<std::uint8_t>((argb >> 8) & 0xFFu)),
          B(static_cast<std::uint8_t>(argb & 0xFFu)) {}

    constexpr std::uint32_t ToArgb() const noexcept {
        return (static_cast<std::uint32_t>(A) << 24) |
               (static_cast<std::uint32_t>(R) << 16) |
               (static_cast<std::uint32_t>(G) << 8) |
               static_cast<std::uint32_t>(B);
    }

    static constexpr Color FromArgb(std::uint32_t argb) noexcept {
        return Color(argb);
    }

    static constexpr Color FromRgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept {
        return Color(0xFF, r, g, b);
    }

    constexpr bool operator==(const Color& other) const noexcept {
        return A == other.A && R == other.R && G == other.G && B == other.B;
    }

    constexpr bool operator!=(const Color& other) const noexcept {
        return !(*this == other);
    }
};

}