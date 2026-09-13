/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

namespace NeoWPF::UI {

struct Thickness {
    int Left = 0;
    int Top = 0;
    int Right = 0;
    int Bottom = 0;

    constexpr Thickness() noexcept = default;

    constexpr Thickness(int uniform) noexcept
        : Left(uniform), Top(uniform), Right(uniform), Bottom(uniform) {}

    constexpr Thickness(int left, int top, int right, int bottom) noexcept
        : Left(left), Top(top), Right(right), Bottom(bottom) {}

    constexpr int Horizontal() const noexcept { return Left + Right; }
    constexpr int Vertical() const noexcept { return Top + Bottom; }

    constexpr bool operator==(const Thickness& other) const noexcept {
        return Left == other.Left && Top == other.Top && Right == other.Right && Bottom == other.Bottom;
    }

    constexpr bool operator!=(const Thickness& other) const noexcept {
        return !(*this == other);
    }
};

}
