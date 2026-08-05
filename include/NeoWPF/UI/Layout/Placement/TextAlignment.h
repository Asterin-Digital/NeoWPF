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
};

}
