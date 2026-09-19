/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

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
        double availableWidth,
        double availableHeight,
        HorizontalAlignment hAlign,
        VerticalAlignment vAlign,
        double& outX,
        double& outY,
        double& outWidth,
        double& outHeight
    );
};

}
