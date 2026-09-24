/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <algorithm>

namespace NeoWPF::UI {

enum class TextHorizontalAlignment {
    Left,
    Center,
    Right
};

enum class TextVerticalAlignment {
    Top,
    Center,
    Bottom
};

class TextAlignment {
public:
    static void ApplyTextAlignment(
        double contentWidth,
        double contentHeight,
        double availableWidth,
        double availableHeight,
        TextHorizontalAlignment horizontalAlignment,
        TextVerticalAlignment verticalAlignment,
        double& outX,
        double& outY) noexcept;

    static double GetBlockVerticalOffset(double contentWidth, double contentHeight, double viewportHeight,
                                         TextVerticalAlignment alignment) noexcept;

    static double GetLineHorizontalOffset(double lineWidth, double viewportWidth,
                                         TextHorizontalAlignment alignment) noexcept;

    static void ApplyTextAlignmentF(
        double contentWidth,
        double contentHeight,
        double availableWidth,
        double availableHeight,
        TextHorizontalAlignment horizontalAlignment,
        TextVerticalAlignment verticalAlignment,
        float& outX,
        float& outY) noexcept;

    static float GetBlockVerticalOffsetF(double contentWidth, double contentHeight, double viewportHeight,
                                         TextVerticalAlignment alignment) noexcept;

    static float GetLineHorizontalOffsetF(double lineWidth, double viewportWidth,
                                          TextHorizontalAlignment alignment) noexcept;
};

}
