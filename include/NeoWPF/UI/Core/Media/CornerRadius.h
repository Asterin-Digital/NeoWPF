#pragma once

namespace NeoWPF::UI {

struct CornerRadius {
    float TopLeft = 0.0f;
    float TopRight = 0.0f;
    float BottomRight = 0.0f;
    float BottomLeft = 0.0f;

    constexpr CornerRadius() noexcept = default;

    constexpr CornerRadius(float uniformRadius) noexcept
        : TopLeft(uniformRadius), TopRight(uniformRadius), BottomRight(uniformRadius), BottomLeft(uniformRadius) {}

    constexpr CornerRadius(float topLeft, float topRight, float bottomRight, float bottomLeft) noexcept
        : TopLeft(topLeft), TopRight(topRight), BottomRight(bottomRight), BottomLeft(bottomLeft) {}

    bool operator==(const CornerRadius& other) const noexcept {
        return TopLeft == other.TopLeft && TopRight == other.TopRight &&
               BottomRight == other.BottomRight && BottomLeft == other.BottomLeft;
    }

    bool operator!=(const CornerRadius& other) const noexcept {
        return !(*this == other);
    }
};

}