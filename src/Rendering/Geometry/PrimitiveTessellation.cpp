#include <NeoWPF/Rendering/Geometry/PrimitiveTessellation.h>

#include <algorithm>
#include <array>
#include <cmath>

namespace NeoWPF::Rendering {
namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr std::size_t kRoundedOutlineVertices = 41;

enum class ClipEdge {
    Left,
    Right,
    Top,
    Bottom,
};

double GetEdgeCoordinate(const GeometryVertex& vertex, ClipEdge edge) noexcept
{
    return edge == ClipEdge::Left || edge == ClipEdge::Right ? vertex.x : vertex.y;
}

bool IsInsideClipEdge(const GeometryVertex& vertex, ClipEdge edge, double boundary) noexcept
{
    switch (edge) {
    case ClipEdge::Left:
    case ClipEdge::Top:
        return GetEdgeCoordinate(vertex, edge) >= boundary;
    case ClipEdge::Right:
    case ClipEdge::Bottom:
        return GetEdgeCoordinate(vertex, edge) <= boundary;
    }
    return false;
}

GeometryVertex IntersectClipEdge(
    const GeometryVertex& start,
    const GeometryVertex& end,
    ClipEdge edge,
    double boundary) noexcept
{
    const double startCoordinate = GetEdgeCoordinate(start, edge);
    const double endCoordinate = GetEdgeCoordinate(end, edge);
    const double amount = (boundary - startCoordinate) / (endCoordinate - startCoordinate);
    GeometryVertex intersection{
        start.x + (end.x - start.x) * amount,
        start.y + (end.y - start.y) * amount,
        start.u + (end.u - start.u) * static_cast<float>(amount),
        start.v + (end.v - start.v) * static_cast<float>(amount),
    };
    if (edge == ClipEdge::Left || edge == ClipEdge::Right) {
        intersection.x = boundary;
    } else {
        intersection.y = boundary;
    }
    return intersection;
}

void ClipPolygonToEdge(
    std::array<GeometryVertex, 8>& polygon,
    std::size_t& pointCount,
    ClipEdge edge,
    double boundary) noexcept
{
    if (pointCount == 0) {
        return;
    }

    std::array<GeometryVertex, 8> clipped{};
    std::size_t clippedCount = 0;
    GeometryVertex previous = polygon[pointCount - 1];
    bool previousInside = IsInsideClipEdge(previous, edge, boundary);
    for (std::size_t index = 0; index < pointCount; ++index) {
        const GeometryVertex current = polygon[index];
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

bool IsValidClip(const ClipRect& clip) noexcept
{
    return !clip.active || (clip.w > 0.0 && clip.h > 0.0 &&
        std::isfinite(clip.x) && std::isfinite(clip.y) &&
        std::isfinite(clip.w) && std::isfinite(clip.h) &&
        std::isfinite(clip.x + clip.w) && std::isfinite(clip.y + clip.h));
}

void AppendTriangle(
    std::vector<GeometryVertex>& vertices,
    GeometryVertex first,
    GeometryVertex second,
    GeometryVertex third,
    const ClipRect& clip)
{
    if (!clip.active) {
        vertices.insert(vertices.end(), {first, second, third});
        return;
    }
    if (!IsValidClip(clip)) {
        return;
    }

    std::array<GeometryVertex, 8> polygon{first, second, third};
    std::size_t pointCount = 3;
    ClipPolygonToEdge(polygon, pointCount, ClipEdge::Left, clip.x);
    ClipPolygonToEdge(polygon, pointCount, ClipEdge::Right, clip.x + clip.w);
    ClipPolygonToEdge(polygon, pointCount, ClipEdge::Top, clip.y);
    ClipPolygonToEdge(polygon, pointCount, ClipEdge::Bottom, clip.y + clip.h);
    for (std::size_t index = 1; index + 1 < pointCount; ++index) {
        vertices.insert(vertices.end(), {polygon[0], polygon[index], polygon[index + 1]});
    }
}

void AppendPolygon(
    std::vector<GeometryVertex>& vertices,
    GeometryVertex* polygon,
    std::size_t pointCount,
    const ClipRect& clip,
    float rotationRadians,
    double pivotX,
    double pivotY)
{
    if (pointCount < 3 || !IsValidClip(clip)) {
        return;
    }

    if (std::fabs(rotationRadians) > 1e-6f) {
        const double cosine = std::cos(rotationRadians);
        const double sine = std::sin(rotationRadians);
        for (std::size_t index = 0; index < pointCount; ++index) {
            auto& vertex = polygon[index];
            const double relativeX = vertex.x - pivotX;
            const double relativeY = vertex.y - pivotY;
            vertex.x = pivotX + relativeX * cosine - relativeY * sine;
            vertex.y = pivotY + relativeX * sine + relativeY * cosine;
        }
    }

    for (std::size_t index = 1; index + 1 < pointCount; ++index) {
        AppendTriangle(vertices, polygon[0], polygon[index], polygon[index + 1], clip);
    }
}

void AddCornerArc(
    std::array<GeometryVertex, kRoundedOutlineVertices>& outline,
    std::size_t& pointCount,
    double centerX,
    double centerY,
    double startAngle,
    double endAngle,
    double radius)
{
    for (int index = 0; index <= 8; ++index) {
        const double amount = static_cast<double>(index) / 8.0;
        const double angle = startAngle + (endAngle - startAngle) * amount;
        outline[pointCount++] = {centerX + radius * std::cos(angle),
                     centerY + radius * std::sin(angle), 0.0f, 0.0f};
    }
}

}

void AppendClippedTriangle(
    std::vector<GeometryVertex>& vertices,
    GeometryVertex first,
    GeometryVertex second,
    GeometryVertex third,
    const ClipRect& clip)
{
    AppendTriangle(vertices, first, second, third, clip);
}

void TessellateRect(
    std::vector<GeometryVertex>& vertices,
    double x,
    double y,
    double width,
    double height,
    const ClipRect& clip,
    float rotationRadians,
    double pivotX,
    double pivotY,
    float u0,
    float v0,
    float u1,
    float v1)
{
    vertices.clear();
    if (width <= 0.0 || height <= 0.0 || !std::isfinite(x) || !std::isfinite(y) ||
        !std::isfinite(width) || !std::isfinite(height)) {
        return;
    }

    std::array<GeometryVertex, 4> polygon{{
        {x, y, u0, v0},
        {x + width, y, u1, v0},
        {x + width, y + height, u1, v1},
        {x, y + height, u0, v1},
    }};
    AppendPolygon(vertices, polygon.data(), polygon.size(), clip,
                  rotationRadians, pivotX, pivotY);
}

void TessellateRoundedRect(
    std::vector<GeometryVertex>& vertices,
    double x,
    double y,
    double width,
    double height,
    double topLeft,
    double topRight,
    double bottomRight,
    double bottomLeft,
    const ClipRect& clip,
    float rotationRadians,
    double pivotX,
    double pivotY)
{
    vertices.clear();
    if (width <= 0.0 || height <= 0.0 || !std::isfinite(x) || !std::isfinite(y) ||
        !std::isfinite(width) || !std::isfinite(height) || !IsValidClip(clip)) {
        return;
    }

    const double radiusLimit = (std::min)(width * 0.5, height * 0.5);
    const auto clampRadius = [radiusLimit](double radius) {
        return (std::clamp)(radius, 0.0, radiusLimit);
    };
    const double topLeftRadius = clampRadius(topLeft);
    const double topRightRadius = clampRadius(topRight);
    const double bottomRightRadius = clampRadius(bottomRight);
    const double bottomLeftRadius = clampRadius(bottomLeft);

    std::array<GeometryVertex, kRoundedOutlineVertices> outline{};
    std::size_t pointCount = 0;
    outline[pointCount++] = {x + topLeftRadius, y, 0.0f, 0.0f};
    outline[pointCount++] = {x + width - topRightRadius, y, 0.0f, 0.0f};
    AddCornerArc(outline, pointCount, x + width - topRightRadius, y + topRightRadius,
        -kPi * 0.5, 0.0, topRightRadius);
    outline[pointCount++] = {x + width, y + height - bottomRightRadius, 0.0f, 0.0f};
    AddCornerArc(outline, pointCount, x + width - bottomRightRadius, y + height - bottomRightRadius,
        0.0, kPi * 0.5, bottomRightRadius);
    outline[pointCount++] = {x + bottomLeftRadius, y + height, 0.0f, 0.0f};
    AddCornerArc(outline, pointCount, x + bottomLeftRadius, y + height - bottomLeftRadius,
        kPi * 0.5, kPi, bottomLeftRadius);
    outline[pointCount++] = {x, y + topLeftRadius, 0.0f, 0.0f};
    AddCornerArc(outline, pointCount, x + topLeftRadius, y + topLeftRadius,
        kPi, kPi * 1.5, topLeftRadius);
    AppendPolygon(vertices, outline.data(), pointCount, clip,
                  rotationRadians, pivotX, pivotY);
}

}