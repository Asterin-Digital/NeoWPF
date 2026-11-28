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

#include <limits>

namespace NeoWPF::UI::Layout {

inline constexpr double Unbounded = std::numeric_limits<double>::infinity();

/*
    @summary
    Reports whether a layout size is a finite constraint.
*/
inline bool IsBounded(double size) noexcept
{
    return std::numeric_limits<double>::has_infinity && size < Unbounded;
}

}
