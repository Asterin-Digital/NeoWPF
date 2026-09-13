/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

namespace NeoWPF::Rendering {

struct ClipRect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
    bool active = false;
};

 

bool ClipRectToBounds(float x, float y, float w, float h, const ClipRect& clip, float& outX, float& outY, float& outW, float& outH) noexcept;

}
