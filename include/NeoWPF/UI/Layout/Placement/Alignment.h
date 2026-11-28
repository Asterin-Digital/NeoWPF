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
