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
    /// Applies alignment to a child element within available space
    /// @param child The child element being arranged
    /// @param availableWidth Total width available for the child
    /// @param availableHeight Total height available for the child
    /// @param hAlign Horizontal alignment preference
    /// @param vAlign Vertical alignment preference
    /// @param outX Output: X position for the child
    /// @param outY Output: Y position for the child
    /// @param outWidth Output: Final width for the child
    /// @param outHeight Output: Final height for the child
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
