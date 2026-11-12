/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Rendering/Geometry/PrimitiveTessellation.h>

#include <vector>

namespace NeoWPF::Rendering {

/*
    @summary
    Tessellates an annular arc into clipped triangle-list vertices.

    @param vertices
    The output triangle-list vertices. Existing contents are replaced.

    @param centerX
    The horizontal coordinate of the arc center.

    @param centerY
    The vertical coordinate of the arc center.

    @param radius
    The distance from the center to the middle of the arc thickness.

    @param thickness
    The radial thickness of the arc.

    @param startAngleRadians
    The clockwise angle from the top of the circle where the arc begins.

    @param sweepAngleRadians
    The signed sweep angle, clamped to one revolution in either direction.

    @param clip
    The clipping rectangle.

    @internal
*/
void TessellateAnnularArc(
    std::vector<GeometryVertex>& vertices,
    double centerX,
    double centerY,
    double radius,
    double thickness,
    double startAngleRadians,
    double sweepAngleRadians,
    const ClipRect& clip);

}