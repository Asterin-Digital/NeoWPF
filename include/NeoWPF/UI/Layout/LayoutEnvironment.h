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

#include <NeoWPF/UI/Core/Media/Thickness.h>

namespace NeoWPF::UI::Layout {

inline constexpr double TouchMinTargetSize = 48.0;

/*
    @summary
    Describes coarse window layout facts used by responsive rules.
*/
enum class PointerPrecision {
    Fine,
    Coarse
};

/*
    @summary
    Stores window environment facts in device-independent units.
*/
struct LayoutEnvironment {
    double contentWidth = 0.0;
    double contentHeight = 0.0;
    Thickness safeArea;
    Thickness keyboard;
    PointerPrecision pointer = PointerPrecision::Fine;
    bool hover = true;
    double scale = 1.0;
};

}
