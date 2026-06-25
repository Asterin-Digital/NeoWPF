#pragma once

#include <Windows.h>
#include <vector>
#include <cstdint>

namespace NeoWPF::Rendering {

struct CommandList {
    struct DrawRect { float x; float y; float w; float h; uint32_t color; };
    std::vector<DrawRect> rects;
    struct DrawTexturedQuad { float x; float y; float w; float h; float u0; float v0; float u1; float v1; uint32_t textureId; };
    std::vector<DrawTexturedQuad> texturedQuads;
    void Clear() { rects.clear(); texturedQuads.clear(); }
    void DrawRect(float x, float y, float w, float h, uint32_t color) { rects.push_back({x,y,w,h,color}); }
    void DrawTexturedQuad(float x, float y, float w, float h, float u0, float v0, float u1, float v1, uint32_t tex) { texturedQuads.push_back({x,y,w,h,u0,v0,u1,v1,tex}); }
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
