#include <NeoWPF/Rendering/Clipping.h>

#include <algorithm>
#include <cmath>

namespace NeoWPF::Rendering {

bool ClipRectToBounds(float x, float y, float w, float h, const ClipRect& clip,
                      float& outX, float& outY, float& outW, float& outH) noexcept
{
    outX = x;
    outY = y;
    outW = w;
    outH = h;

    if (!clip.active) {
        return false;
    }

    if (w <= 0.0f || h <= 0.0f || clip.w <= 0.0f || clip.h <= 0.0f ||
        !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(w) || !std::isfinite(h) ||
        !std::isfinite(clip.x) || !std::isfinite(clip.y) ||
        !std::isfinite(clip.w) || !std::isfinite(clip.h)) {
        outX = outY = outW = outH = 0.0f;
        return true;
    }

    const float intersectLeft = (std::max)(x, clip.x);
    const float intersectTop = (std::max)(y, clip.y);
    const float intersectRight = (std::min)(x + w, clip.x + clip.w);
    const float intersectBottom = (std::min)(y + h, clip.y + clip.h);

    if (intersectRight <= intersectLeft || intersectBottom <= intersectTop) {
        outX = outY = outW = outH = 0.0f;
        return true;
    }

    outX = intersectLeft;
    outY = intersectTop;
    outW = intersectRight - intersectLeft;
    outH = intersectBottom - intersectTop;
    return true;
}

}