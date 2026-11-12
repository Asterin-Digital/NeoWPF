/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

namespace NeoWPF::Rendering {

/*
    @summary
    Describes an axis-aligned clipping rectangle.

    @internal
*/
struct ClipRect {
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;
    bool active = false;
};

/*
    @summary
    Intersects a rectangle with a clipping rectangle.

    @param x
    The rectangle's horizontal position.

    @param y
    The rectangle's vertical position.

    @param w
    The rectangle width.

    @param h
    The rectangle height.

    @param clip
    The clipping rectangle.

    @param outX
    Receives the visible horizontal position.

    @param outY
    Receives the visible vertical position.

    @param outW
    Receives the visible width.

    @param outH
    Receives the visible height.

    @returns
    True when clipping was active.

    @internal
*/
bool ClipRectToBounds(
    double x,
    double y,
    double w,
    double h,
    const ClipRect& clip,
    double& outX,
    double& outY,
    double& outW,
    double& outH) noexcept;

}