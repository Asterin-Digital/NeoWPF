#include <NeoWPF/Rendering/Clipping.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>
#include <cmath>

namespace NeoWPF::Rendering {

bool s_clippingLogging = false;

bool ClipRectToBounds(float x, float y, float w, float h, const ClipRect& clip,
                      float& outX, float& outY, float& outW, float& outH) noexcept {
    outX = x;
    outY = y;
    outW = w;
    outH = h;

    if (!clip.active) {
        return false;
    }

    if (w <= 0.0f || h <= 0.0f || clip.w <= 0.0f || clip.h <= 0.0f) {
        return false;
    }

    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(w) || !std::isfinite(h) ||
        !std::isfinite(clip.x) || !std::isfinite(clip.y) || !std::isfinite(clip.w) || !std::isfinite(clip.h)) {
        return false;
    }

    const float clipLeft = clip.x;
    const float clipTop = clip.y;
    const float clipRight = clipLeft + clip.w;
    const float clipBottom = clipTop + clip.h;

    const float intersectLeft = std::max(x, clipLeft);
    const float intersectTop = std::max(y, clipTop);
    const float intersectRight = std::min(x + w, clipRight);
    const float intersectBottom = std::min(y + h, clipBottom);

    if (intersectRight <= intersectLeft || intersectBottom <= intersectTop) {
        outX = 0.0f;
        outY = 0.0f;
        outW = 0.0f;
        outH = 0.0f;
        return true;
    }

    outX = intersectLeft;
    outY = intersectTop;
    outW = intersectRight - intersectLeft;
    outH = intersectBottom - intersectTop;
    return true;
}

}