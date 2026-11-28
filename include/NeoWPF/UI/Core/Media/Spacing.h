/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

#pragma once

namespace NeoWPF::UI {

struct Spacing {
    double Horizontal = 0.0;
    double Vertical = 0.0;

    constexpr Spacing() noexcept = default;

    constexpr Spacing(double uniform) noexcept
        : Horizontal(uniform), Vertical(uniform) {}

    constexpr Spacing(double horizontal, double vertical) noexcept
        : Horizontal(horizontal), Vertical(vertical) {}

    constexpr bool operator==(const Spacing& other) const noexcept {
        return Horizontal == other.Horizontal && Vertical == other.Vertical;
    }

    constexpr bool operator!=(const Spacing& other) const noexcept {
        return !(*this == other);
    }
};

}
