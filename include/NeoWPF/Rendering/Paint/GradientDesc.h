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

#include <array>
#include <cstdint>

namespace NeoWPF::Rendering {

enum class GradientSpreadMethod : std::uint8_t {
    Pad,
    Reflect,
    Repeat,
};

struct GradientStopDesc {
    float offset = 0.0f;
    std::uint32_t color = 0;
};

struct GradientDesc {
    enum class Kind : std::uint8_t {
        Linear,
        Radial,
    };

    Kind kind = Kind::Linear;
    GradientSpreadMethod spread = GradientSpreadMethod::Pad;
    std::array<double, 6> mapping{};
    double focalX = 0.5;
    double focalY = 0.5;
    float opacity = 1.0f;
    std::uint32_t stopOffset = 0;
    std::uint32_t stopCount = 0;
};

}
