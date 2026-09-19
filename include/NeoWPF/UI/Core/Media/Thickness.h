/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

namespace NeoWPF::UI {

struct Thickness {
    double Left = 0.0;
    double Top = 0.0;
    double Right = 0.0;
    double Bottom = 0.0;

    constexpr Thickness() noexcept = default;

    constexpr Thickness(double uniform) noexcept
        : Left(uniform), Top(uniform), Right(uniform), Bottom(uniform) {}

    constexpr Thickness(double left, double top, double right, double bottom) noexcept
        : Left(left), Top(top), Right(right), Bottom(bottom) {}

    constexpr double Horizontal() const noexcept { return Left + Right; }
    constexpr double Vertical() const noexcept { return Top + Bottom; }

    constexpr bool operator==(const Thickness& other) const noexcept {
        return Left == other.Left && Top == other.Top && Right == other.Right && Bottom == other.Bottom;
    }

    constexpr bool operator!=(const Thickness& other) const noexcept {
        return !(*this == other);
    }
};

}
