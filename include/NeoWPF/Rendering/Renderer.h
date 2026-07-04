#pragma once

#include <Windows.h>
#include <vector>
#include <cstdint>

namespace NeoWPF::Rendering {

struct CommandList {
    struct DrawRect { float x; float y; float w; float h; uint32_t color; float opacity; };
    std::vector<DrawRect> rects;
    struct DrawTexturedQuad { float x; float y; float w; float h; float u0; float v0; float u1; float v1; uint32_t textureId; float opacity; };
    std::vector<DrawTexturedQuad> texturedQuads;
    void Clear() { rects.clear(); texturedQuads.clear(); }
    void DrawRect(float x, float y, float w, float h, uint32_t color, float opacity = 1.0f) { rects.push_back({x,y,w,h,color,opacity}); }
    void DrawTexturedQuad(float x, float y, float w, float h, float u0, float v0, float u1, float v1, uint32_t tex, float opacity = 1.0f) { texturedQuads.push_back({x,y,w,h,u0,v0,u1,v1,tex,opacity}); }
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
