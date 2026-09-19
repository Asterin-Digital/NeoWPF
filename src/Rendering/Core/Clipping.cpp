#include <NeoWPF/Rendering/Clipping.h>

#include <algorithm>
#include <cmath>

namespace NeoWPF::Rendering {

bool ClipRectToBounds(double x, double y, double w, double h, const ClipRect& clip,
                      double& outX, double& outY, double& outW, double& outH) noexcept
{
    outX = x;
    outY = y;
    outW = w;
    outH = h;

    if (!clip.active) {
        return false;
    }

    if (w <= 0.0 || h <= 0.0 || clip.w <= 0.0 || clip.h <= 0.0 ||
        !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(w) || !std::isfinite(h) ||
        !std::isfinite(clip.x) || !std::isfinite(clip.y) ||
        !std::isfinite(clip.w) || !std::isfinite(clip.h)) {
        outX = outY = outW = outH = 0.0;
        return true;
    }

    const double intersectLeft = (std::max)(x, clip.x);
    const double intersectTop = (std::max)(y, clip.y);
    const double intersectRight = (std::min)(x + w, clip.x + clip.w);
    const double intersectBottom = (std::min)(y + h, clip.y + clip.h);

    if (intersectRight <= intersectLeft || intersectBottom <= intersectTop) {
        outX = outY = outW = outH = 0.0;
        return true;
    }

    outX = intersectLeft;
    outY = intersectTop;
    outW = intersectRight - intersectLeft;
    outH = intersectBottom - intersectTop;
    return true;
}

}