/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Rendering/Paint/GradientDesc.h>

#include <cstddef>
#include <cstdint>
#include <span>

namespace NeoWPF::Rendering {

constexpr std::uint32_t kGradientRampWidth = 256;
constexpr std::uint32_t kGradientRampHeight = 256;

/*
    @summary
    Bakes gradient stops into a compact color ramp.

    @param stops
    The gradient stops to sample.

    @param output
    Output RGBA pixels.

    @param width
    Number of pixels to write.

    @internal
*/
void BakeRamp(std::span<const GradientStopDesc> stops, std::span<std::uint32_t> output, std::size_t width);

/*
    @summary
    Computes a cache key for gradient and stop data in a ramp atlas.

    @internal
*/
std::uint64_t ComputeGradientAtlasHash(
    std::span<const GradientDesc> gradients,
    std::span<const GradientStopDesc> stops) noexcept;


}
