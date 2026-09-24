/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

namespace NeoWPF::Rendering {

struct ClipRect {
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;
    bool active = false;
};

 

bool ClipRectToBounds(double x, double y, double w, double h, const ClipRect& clip, double& outX, double& outY, double& outW, double& outH) noexcept;

}
