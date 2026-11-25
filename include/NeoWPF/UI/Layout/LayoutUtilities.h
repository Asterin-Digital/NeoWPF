/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
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
