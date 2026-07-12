#pragma once

#include <NeoWPF/Rendering/Clipping.h>
#include <Windows.h>
#include <vector>
#include <cstdint>

namespace NeoWPF::Rendering {

struct CommandList {
    struct DrawRect { float x; float y; float w; float h; uint32_t color; float opacity; ClipRect clip; float rotationRadians = 0.0f; };
    std::vector<DrawRect> rects;
    struct DrawRoundedRect { float x; float y; float w; float h; float topLeft; float topRight; float bottomRight; float bottomLeft; uint32_t color; float opacity; ClipRect clip; float rotationRadians = 0.0f; };
    std::vector<DrawRoundedRect> roundedRects;
    struct DrawTexturedQuad { float x; float y; float w; float h; float u0; float v0; float u1; float v1; uint32_t textureId; float opacity; ClipRect clip; };
    std::vector<DrawTexturedQuad> texturedQuads;
    void Clear() { rects.clear(); roundedRects.clear(); texturedQuads.clear(); }
    void DrawRect(float x, float y, float w, float h, uint32_t color, float opacity = 1.0f, const ClipRect& clip = ClipRect{}, float rotationRadians = 0.0f) { rects.push_back({x,y,w,h,color,opacity,clip,rotationRadians}); }
    void DrawRoundedRect(float x, float y, float w, float h, float topLeft, float topRight, float bottomRight, float bottomLeft, uint32_t color, float opacity = 1.0f, const ClipRect& clip = ClipRect{}, float rotationRadians = 0.0f) { roundedRects.push_back({x,y,w,h,topLeft,topRight,bottomRight,bottomLeft,color,opacity,clip,rotationRadians}); }
    void DrawTexturedQuad(float x, float y, float w, float h, float u0, float v0, float u1, float v1, uint32_t tex, float opacity = 1.0f, const ClipRect& clip = ClipRect{}) { texturedQuads.push_back({x,y,w,h,u0,v0,u1,v1,tex,opacity,clip}); }
};

void Renderer_Initialize(HWND hwnd);
void Renderer_Shutdown();
void Renderer_Resize(unsigned int w, unsigned int h);
void Renderer_BeginFrame();
void Renderer_Execute(const CommandList& cmd);
void Renderer_Present();
uint32_t Renderer_CreateTexture(unsigned int w, unsigned int h, const void* rgbaData);
void Renderer_DestroyTexture(uint32_t id);

}
