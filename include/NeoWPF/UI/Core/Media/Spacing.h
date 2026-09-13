/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

namespace NeoWPF::UI {

struct Spacing {
    int Horizontal = 0;
    int Vertical = 0;

    constexpr Spacing() noexcept = default;

    constexpr Spacing(int uniform) noexcept
        : Horizontal(uniform), Vertical(uniform) {}

    constexpr Spacing(int horizontal, int vertical) noexcept
        : Horizontal(horizontal), Vertical(vertical) {}

    constexpr bool operator==(const Spacing& other) const noexcept {
        return Horizontal == other.Horizontal && Vertical == other.Vertical;
    }

    constexpr bool operator!=(const Spacing& other) const noexcept {
        return !(*this == other);
    }
};

}
