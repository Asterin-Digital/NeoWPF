/*
MIT License — Copyright (c) 2026 NeoWPF
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
        int contentWidth,
        int contentHeight,
        int availableWidth,
        int availableHeight,
        TextHorizontalAlignment horizontalAlignment,
        TextVerticalAlignment verticalAlignment,
        int& outX,
        int& outY) noexcept;

    static int GetBlockVerticalOffset(int contentWidth, int contentHeight, int viewportHeight,
                                      TextVerticalAlignment alignment) noexcept;

    static int GetLineHorizontalOffset(int lineWidth, int viewportWidth,
                                      TextHorizontalAlignment alignment) noexcept;
};

}
