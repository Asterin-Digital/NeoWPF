#pragma once

namespace NeoWPF::UI {

enum class HorizontalAlignment {
    Left,
    Center,
    Right,
    Stretch
};

enum class VerticalAlignment {
    Top,
    Center,
    Bottom,
    Stretch
};

class FrameworkElement;

class Alignment {
public:
    static void ApplyAlignment(
        FrameworkElement* child,
        int availableWidth,
        int availableHeight,
        HorizontalAlignment hAlign,
        VerticalAlignment vAlign,
        int& outX,
        int& outY,
        int& outWidth,
        int& outHeight
    );
};

} // namespace NeoWPF::UI
