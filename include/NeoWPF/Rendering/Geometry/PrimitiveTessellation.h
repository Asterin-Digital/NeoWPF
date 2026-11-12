/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Rendering/Geometry/Clipping.h>

#include <vector>

namespace NeoWPF::Rendering {

/*
    @summary
    Represents a tessellated vertex with optional texture coordinates.

    @internal
*/
struct GeometryVertex {
    double x;
    double y;
    float u;
    float v;
};

/*
    @summary
    Appends a triangle clipped to an axis-aligned rectangle.

    @param vertices
    The output triangle-list vertices.

    @param first
    The first triangle vertex.

    @param second
    The second triangle vertex.

    @param third
    The third triangle vertex.

    @param clip
    The clipping rectangle.

    @internal
*/
void AppendClippedTriangle(
    std::vector<GeometryVertex>& vertices,
    GeometryVertex first,
    GeometryVertex second,
    GeometryVertex third,
    const ClipRect& clip);

/*
    @summary
    Tessellates and clips a rectangle into triangle-list vertices.

    @param vertices
    The output triangle-list vertices. Existing contents are replaced.

    @param x
    The left position of the rectangle.

    @param y
    The top position of the rectangle.

    @param width
    The rectangle width.

    @param height
    The rectangle height.

    @param clip
    The clipping rectangle.

    @param rotationRadians
    The rotation applied before clipping.

    @param pivotX
    The horizontal rotation pivot.

    @param pivotY
    The vertical rotation pivot.

    @param u0
    The left texture coordinate.

    @param v0
    The top texture coordinate.

    @param u1
    The right texture coordinate.

    @param v1
    The bottom texture coordinate.

    @internal
*/
void TessellateRect(
    std::vector<GeometryVertex>& vertices,
    double x,
    double y,
    double width,
    double height,
    const ClipRect& clip,
    float rotationRadians = 0.0f,
    double pivotX = 0.0,
    double pivotY = 0.0,
    float u0 = 0.0f,
    float v0 = 0.0f,
    float u1 = 1.0f,
    float v1 = 1.0f);

/*
    @summary
    Tessellates and clips a rounded rectangle into triangle-list vertices.

    @param vertices
    The output triangle-list vertices. Existing contents are replaced.

    @param x
    The left position of the rectangle.

    @param y
    The top position of the rectangle.

    @param width
    The rectangle width.

    @param height
    The rectangle height.

    @param topLeft
    The top-left corner radius.

    @param topRight
    The top-right corner radius.

    @param bottomRight
    The bottom-right corner radius.

    @param bottomLeft
    The bottom-left corner radius.

    @param clip
    The clipping rectangle.

    @param rotationRadians
    The rotation applied before clipping.

    @param pivotX
    The horizontal rotation pivot.

    @param pivotY
    The vertical rotation pivot.

    @internal
*/
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
    float rotationRadians = 0.0f,
    double pivotX = 0.0,
    double pivotY = 0.0);

}