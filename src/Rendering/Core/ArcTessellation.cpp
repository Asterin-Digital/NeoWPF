#include <NeoWPF/Rendering/ArcTessellation.h>

#include <algorithm>
#include <array>
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

enum class ClipEdge {
    Left,
    Right,
    Top,
    Bottom,
};

double GetEdgeCoordinate(const Point2& point, ClipEdge edge) noexcept
{
    return edge == ClipEdge::Left || edge == ClipEdge::Right ? point.x : point.y;
}

bool IsInsideClipEdge(const Point2& point, ClipEdge edge, double boundary) noexcept
{
    switch (edge) {
    case ClipEdge::Left:
    case ClipEdge::Top:
        return GetEdgeCoordinate(point, edge) >= boundary;
    case ClipEdge::Right:
    case ClipEdge::Bottom:
        return GetEdgeCoordinate(point, edge) <= boundary;
    }
    return false;
}

Point2 IntersectClipEdge(const Point2& start, const Point2& end, ClipEdge edge, double boundary) noexcept
{
    const double startCoordinate = GetEdgeCoordinate(start, edge);
    const double endCoordinate = GetEdgeCoordinate(end, edge);
    const double amount = (boundary - startCoordinate) / (endCoordinate - startCoordinate);
    Point2 intersection{
        start.x + (end.x - start.x) * amount,
        start.y + (end.y - start.y) * amount,
    };
    if (edge == ClipEdge::Left || edge == ClipEdge::Right) {
        intersection.x = boundary;
    } else {
        intersection.y = boundary;
    }
    return intersection;
}

void ClipPolygonToEdge(
    std::array<Point2, 8>& polygon,
    std::size_t& pointCount,
    ClipEdge edge,
    double boundary) noexcept
{
    if (pointCount == 0) {
        return;
    }

    std::array<Point2, 8> clipped{};
    std::size_t clippedCount = 0;
    Point2 previous = polygon[pointCount - 1];
    bool previousInside = IsInsideClipEdge(previous, edge, boundary);
    for (std::size_t index = 0; index < pointCount; ++index) {
        const Point2 current = polygon[index];
        const bool currentInside = IsInsideClipEdge(current, edge, boundary);
        if (currentInside != previousInside) {
            clipped[clippedCount++] = IntersectClipEdge(previous, current, edge, boundary);
        }
        if (currentInside) {
            clipped[clippedCount++] = current;
        }
        previous = current;
        previousInside = currentInside;
    }
    polygon = clipped;
    pointCount = clippedCount;
}

void AppendTriangle(
    std::vector<ArcVertex>& vertices,
    const Point2& first,
    const Point2& second,
    const Point2& third,
    const ClipRect& clip)
{
    if (!clip.active) {
        vertices.push_back({first.x, first.y});
        vertices.push_back({second.x, second.y});
        vertices.push_back({third.x, third.y});
        return;
    }

    std::array<Point2, 8> polygon{first, second, third};
    std::size_t pointCount = 3;
    ClipPolygonToEdge(polygon, pointCount, ClipEdge::Left, clip.x);
    ClipPolygonToEdge(polygon, pointCount, ClipEdge::Right, clip.x + clip.w);
    ClipPolygonToEdge(polygon, pointCount, ClipEdge::Top, clip.y);
    ClipPolygonToEdge(polygon, pointCount, ClipEdge::Bottom, clip.y + clip.h);
    if (pointCount < 3) {
        return;
    }

    for (std::size_t index = 1; index + 1 < pointCount; ++index) {
        vertices.push_back({polygon[0].x, polygon[0].y});
        vertices.push_back({polygon[index].x, polygon[index].y});
        vertices.push_back({polygon[index + 1].x, polygon[index + 1].y});
    }
}

bool IsValidClip(const ClipRect& clip) noexcept
{
    return !clip.active || (clip.w > 0.0 && clip.h > 0.0 &&
        std::isfinite(clip.x) && std::isfinite(clip.y) &&
        std::isfinite(clip.w) && std::isfinite(clip.h) &&
        std::isfinite(clip.x + clip.w) && std::isfinite(clip.y + clip.h));
}

}

std::vector<ArcVertex> TessellateAnnularArc(
    double centerX,
    double centerY,
    double radius,
    double thickness,
    double startAngleRadians,
    double sweepAngleRadians,
    const ClipRect& clip)
{
    std::vector<ArcVertex> vertices;
    if (!std::isfinite(centerX) || !std::isfinite(centerY) ||
        !std::isfinite(radius) || !std::isfinite(thickness) ||
        !std::isfinite(startAngleRadians) || !std::isfinite(sweepAngleRadians) ||
        radius <= 0.0 || thickness <= 0.0 || thickness > 2.0 * radius ||
        !IsValidClip(clip)) {
        return vertices;
    }

    const double halfThickness = thickness * 0.5;
    const double innerRadius = radius - halfThickness;
    const double outerRadius = radius + halfThickness;
    const double maximumFloat = std::numeric_limits<float>::max();
    if (!std::isfinite(outerRadius) ||
        std::abs(centerX) + outerRadius > maximumFloat ||
        std::abs(centerY) + outerRadius > maximumFloat) {
        return vertices;
    }

    const double sweep = (std::max)(-kMaximumSweep, (std::min)(sweepAngleRadians, kMaximumSweep));
    if (sweep == 0.0) {
        return vertices;
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
        const double startRadiansForSegment = startRadians + sweep * startFraction;
        const double endRadiansForSegment = startRadians + sweep * endFraction;

        const Point2 outerStart{
            centerX + outerRadius * std::cos(startRadiansForSegment),
            centerY + outerRadius * std::sin(startRadiansForSegment),
        };
        const Point2 outerEnd{
            centerX + outerRadius * std::cos(endRadiansForSegment),
            centerY + outerRadius * std::sin(endRadiansForSegment),
        };
        const Point2 innerStart{
            centerX + innerRadius * std::cos(startRadiansForSegment),
            centerY + innerRadius * std::sin(startRadiansForSegment),
        };
        const Point2 innerEnd{
            centerX + innerRadius * std::cos(endRadiansForSegment),
            centerY + innerRadius * std::sin(endRadiansForSegment),
        };

        AppendTriangle(vertices, outerStart, outerEnd, innerEnd, clip);
        AppendTriangle(vertices, outerStart, innerEnd, innerStart, clip);
    }

    return vertices;
}

}