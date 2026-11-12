#include <NeoWPF/Rendering/Geometry/ArcTessellation.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace NeoWPF::Rendering {
namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kMaximumSweep = 2.0 * kPi;
constexpr double kMaximumChordError = 0.25;
constexpr std::size_t kMaximumArcSegments = 4096;

struct Point2 {
    double x;
    double y;
};

bool IsValidClip(const ClipRect& clip) noexcept
{
    return !clip.active || (clip.w > 0.0 && clip.h > 0.0 &&
        std::isfinite(clip.x) && std::isfinite(clip.y) &&
        std::isfinite(clip.w) && std::isfinite(clip.h) &&
        std::isfinite(clip.x + clip.w) && std::isfinite(clip.y + clip.h));
}

}

void TessellateAnnularArc(
    std::vector<GeometryVertex>& vertices,
    double centerX,
    double centerY,
    double radius,
    double thickness,
    double startAngleRadians,
    double sweepAngleRadians,
    const ClipRect& clip)
{
    vertices.clear();
    if (!std::isfinite(centerX) || !std::isfinite(centerY) ||
        !std::isfinite(radius) || !std::isfinite(thickness) ||
        !std::isfinite(startAngleRadians) || !std::isfinite(sweepAngleRadians) ||
        radius <= 0.0 || thickness <= 0.0 || thickness > 2.0 * radius ||
        !IsValidClip(clip)) {
        return;
    }

    const double halfThickness = thickness * 0.5;
    const double innerRadius = radius - halfThickness;
    const double outerRadius = radius + halfThickness;
    const double maximumFloat = std::numeric_limits<float>::max();
    if (!std::isfinite(outerRadius) ||
        std::abs(centerX) + outerRadius > maximumFloat ||
        std::abs(centerY) + outerRadius > maximumFloat) {
        return;
    }

    const double sweep = (std::max)(-kMaximumSweep, (std::min)(sweepAngleRadians, kMaximumSweep));
    if (sweep == 0.0) {
        return;
    }

    const double cosineStep = (std::max)(-1.0, (std::min)(1.0, 1.0 - kMaximumChordError / outerRadius));
    const double maximumStep = 2.0 * std::acos(cosineStep);
    std::size_t segmentCount = kMaximumArcSegments;
    if (maximumStep > 0.0 && std::isfinite(maximumStep)) {
        const double requiredSegments = std::ceil(std::abs(sweep) / maximumStep);
        if (requiredSegments < static_cast<double>(kMaximumArcSegments)) {
            segmentCount = (std::max)(std::size_t{1}, static_cast<std::size_t>(requiredSegments));
        }
    }

    vertices.reserve(segmentCount * 6);
    const double startRadians = startAngleRadians - kPi * 0.5;
    for (std::size_t segment = 0; segment < segmentCount; ++segment) {
        const double startFraction = static_cast<double>(segment) / static_cast<double>(segmentCount);
        const double endFraction = static_cast<double>(segment + 1) / static_cast<double>(segmentCount);
        const double segmentStart = startRadians + sweep * startFraction;
        const double segmentEnd = startRadians + sweep * endFraction;
        const Point2 outerStart{centerX + outerRadius * std::cos(segmentStart),
                                centerY + outerRadius * std::sin(segmentStart)};
        const Point2 outerEnd{centerX + outerRadius * std::cos(segmentEnd),
                              centerY + outerRadius * std::sin(segmentEnd)};
        const Point2 innerStart{centerX + innerRadius * std::cos(segmentStart),
                                centerY + innerRadius * std::sin(segmentStart)};
        const Point2 innerEnd{centerX + innerRadius * std::cos(segmentEnd),
                              centerY + innerRadius * std::sin(segmentEnd)};

        AppendClippedTriangle(vertices,
            {outerStart.x, outerStart.y, 0.0f, 0.0f},
            {outerEnd.x, outerEnd.y, 0.0f, 0.0f},
            {innerEnd.x, innerEnd.y, 0.0f, 0.0f}, clip);
        AppendClippedTriangle(vertices,
            {outerStart.x, outerStart.y, 0.0f, 0.0f},
            {innerEnd.x, innerEnd.y, 0.0f, 0.0f},
            {innerStart.x, innerStart.y, 0.0f, 0.0f}, clip);
    }

}

}