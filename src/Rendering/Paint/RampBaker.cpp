#include <NeoWPF/Rendering/Paint/RampBaker.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace NeoWPF::Rendering {
namespace {

std::uint32_t LerpChannel(std::uint32_t left, std::uint32_t right, double amount) noexcept
{
    const double leftValue = static_cast<double>(left);
    const double rightValue = static_cast<double>(right);
    return static_cast<std::uint32_t>(
        std::lround(leftValue + (rightValue - leftValue) * amount));
}

}

void BakeRamp(std::span<const GradientStopDesc> stops,
              std::span<std::uint32_t> output,
              std::size_t width)
{
    if (stops.empty() || output.empty() || width == 0) {
        return;
    }

    std::size_t stopIndex = 0;
    for (std::size_t pixelIndex = 0;
         pixelIndex < output.size() && pixelIndex < width;
         ++pixelIndex) {
        const double position = width == 1
            ? 0.0
            : static_cast<double>(pixelIndex) / static_cast<double>(width - 1);

        while (stopIndex + 1 < stops.size() &&
               position > stops[stopIndex + 1].offset) {
            ++stopIndex;
        }

        if (position <= stops.front().offset) {
            output[pixelIndex] = stops.front().color;
            continue;
        }
        if (position >= stops.back().offset) {
            output[pixelIndex] = stops.back().color;
            continue;
        }

        const auto& left = stops[stopIndex];
        const auto& right = stops[stopIndex + 1];
        const double range = static_cast<double>(right.offset - left.offset);
        const double amount = range > 0.0
            ? (position - left.offset) / range
            : 1.0;
        output[pixelIndex] =
            (LerpChannel((left.color >> 24) & 0xFFu,
                         (right.color >> 24) & 0xFFu, amount) << 24) |
            (LerpChannel((left.color >> 16) & 0xFFu,
                         (right.color >> 16) & 0xFFu, amount) << 16) |
            (LerpChannel((left.color >> 8) & 0xFFu,
                         (right.color >> 8) & 0xFFu, amount) << 8) |
            LerpChannel(left.color & 0xFFu, right.color & 0xFFu, amount);
    }
}

std::uint64_t ComputeGradientAtlasHash(
    std::span<const GradientDesc> gradients,
    std::span<const GradientStopDesc> stops) noexcept
{
    constexpr std::uint64_t offset = 14695981039346656037ull;
    constexpr std::uint64_t prime = 1099511628211ull;
    std::uint64_t hash = offset;
    for (const auto& gradient : gradients) {
        hash ^= static_cast<std::uint64_t>(gradient.kind);
        hash *= prime;
        hash ^= static_cast<std::uint64_t>(gradient.spread);
        hash *= prime;
        hash ^= static_cast<std::uint64_t>(gradient.stopOffset);
        hash *= prime;
        hash ^= static_cast<std::uint64_t>(gradient.stopCount);
        hash *= prime;
        for (const auto value : gradient.mapping) {
            const auto* bytes = reinterpret_cast<const std::byte*>(&value);
            for (std::size_t index = 0; index < sizeof(value); ++index) {
                hash ^= static_cast<std::uint8_t>(bytes[index]);
                hash *= prime;
            }
        }
        hash ^= static_cast<std::uint64_t>(
            static_cast<std::uint32_t>(gradient.opacity * 100000.0f));
        hash *= prime;
        const std::size_t stopOffset =
            gradient.stopOffset < stops.size() ? gradient.stopOffset : stops.size();
        const std::size_t stopCount =
            gradient.stopCount <= stops.size() - stopOffset
                ? gradient.stopCount
                : stops.size() - stopOffset;
        for (std::size_t index = 0; index < stopCount; ++index) {
            const auto& stop = stops[stopOffset + index];
            hash ^= static_cast<std::uint64_t>(stop.offset * 100000.0f);
            hash *= prime;
            hash ^= static_cast<std::uint64_t>(stop.color);
            hash *= prime;
        }
    }
    return hash;
}

}
