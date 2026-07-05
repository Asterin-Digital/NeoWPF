#include <NeoWPF/Rendering/Clipping.h>
#include <algorithm>

namespace NeoWPF::Rendering {

bool ClipRectToBounds(float x, float y, float w, float h, const ClipRect& clip, float& outX, float& outY, float& outW, float& outH) noexcept {
    if (!clip.active || w <= 0.0f || h <= 0.0f) {
        outX = x;
        outY = y;
        outW = w;
        outH = h;
        return false;
    }

    const float clipRight = clip.x + clip.w;
    const float clipBottom = clip.y + clip.h;

    const float clippedX = std::max(x, clip.x);
    const float clippedY = std::max(y, clip.y);
    const float clippedRight = std::min(x + w, clipRight);
    const float clippedBottom = std::min(y + h, clipBottom);

    if (clippedRight <= clippedX || clippedBottom <= clippedY) {
        outX = 0.0f;
        outY = 0.0f;
        outW = 0.0f;
        outH = 0.0f;
        return true;
    }

    outX = clippedX;
    outY = clippedY;
    outW = clippedRight - clippedX;
    outH = clippedBottom - clippedY;
    return true;
}

}
