/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Rendering/Clipping.h>
#include <vector>

namespace NeoWPF::Rendering {

/*
    @summary
    Represents a two-dimensional vertex in tessellated arc geometry.

    @internal
*/
struct ArcVertex {
    double x;
    double y;
};

/*
    @summary
    Tessellates a clipped annular arc into triangle-list vertices.

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
    The clipping rectangle to constrain the generated geometry.

    @returns
    Triangle-list vertices, or an empty vector when the input is invalid or invisible.

    @internal
*/
std::vector<ArcVertex> TessellateAnnularArc(
    double centerX,
    double centerY,
    double radius,
    double thickness,
    double startAngleRadians,
    double sweepAngleRadians,
    const ClipRect& clip);

}