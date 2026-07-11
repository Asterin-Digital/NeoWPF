#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/Clipping.h>
#include <d3d11.h>
#include <wrl.h>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <Windows.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

using Microsoft::WRL::ComPtr;
using namespace NeoWPF::Rendering;

namespace NeoWPF::Rendering::D3D11 {
    extern ComPtr<ID3D11Device> g_device;
    extern ComPtr<ID3D11DeviceContext> g_context;
    extern ComPtr<ID3D11VertexShader> g_vs;
    extern ComPtr<ID3D11PixelShader> g_ps;
    extern ComPtr<ID3D11InputLayout> g_inputLayout;
    extern ComPtr<ID3D11VertexShader> g_vsText;
    extern ComPtr<ID3D11PixelShader> g_psText;
    extern ComPtr<ID3D11InputLayout> g_inputLayoutText;
    extern ComPtr<ID3D11SamplerState> g_samplerState;
    extern ComPtr<ID3D11Buffer> g_vertexBuffer;
    extern ComPtr<ID3D11Buffer> g_constantBuffer;
    unsigned int GetCurrentRendererWidth();
    unsigned int GetCurrentRendererHeight();
    ID3D11RenderTargetView* GetCurrentRendererTargetView();
    ComPtr<ID3D11ShaderResourceView> GetTextureShaderResourceView(uint32_t id);
}

namespace NeoWPF::Rendering::D3D11 {

void ExecuteRendererDraw(const CommandList& cmd)
{
    ID3D11RenderTargetView* rtv = GetCurrentRendererTargetView();
    if (!rtv) return;

    const unsigned int width = GetCurrentRendererWidth();
    const unsigned int height = GetCurrentRendererHeight();

    g_context->OMSetRenderTargets(1, &rtv, nullptr);
    D3D11_VIEWPORT vp = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
    g_context->RSSetViewports(1, &vp);

    float clearColor[4] = {0.125f, 0.125f, 0.125f, 1.0f};
    g_context->ClearRenderTargetView(rtv, clearColor);

    static ComPtr<ID3D11BlendState> blendState;
    if (!blendState) {
        D3D11_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        g_device->CreateBlendState(&blendDesc, &blendState);
    }

    struct Vertex { float x, y; float r, g, b, a; };
    constexpr size_t kVertexBufferCapacity = 1 << 20;
    constexpr size_t kMaxRectVertices = kVertexBufferCapacity / sizeof(Vertex);
    std::vector<Vertex> rectVerts;
    rectVerts.reserve(cmd.rects.size() * 6);

    auto pushRect = [&](float x, float y, float w, float h, float r, float g, float b, float a) {
        rectVerts.push_back({x, y, r, g, b, a});
        rectVerts.push_back({x + w, y, r, g, b, a});
        rectVerts.push_back({x + w, y + h, r, g, b, a});
        rectVerts.push_back({x + w, y + h, r, g, b, a});
        rectVerts.push_back({x, y + h, r, g, b, a});
        rectVerts.push_back({x, y, r, g, b, a});
    };

    auto pushRoundedRect = [&](float x, float y, float w, float h,
        float topLeft, float topRight, float bottomRight, float bottomLeft,
        float r, float g, float b, float a) {
        if (w <= 0.0f || h <= 0.0f) {
            return;
        }

        const auto clampRadius = [](float radius, float maxExtent) {
            return std::max(0.0f, std::min(radius, maxExtent));
        };

        const float tl = clampRadius(topLeft, std::min(w * 0.5f, h * 0.5f));
        const float tr = clampRadius(topRight, std::min(w * 0.5f, h * 0.5f));
        const float br = clampRadius(bottomRight, std::min(w * 0.5f, h * 0.5f));
        const float bl = clampRadius(bottomLeft, std::min(w * 0.5f, h * 0.5f));

        struct Point2 { float x; float y; };
        std::vector<Point2> points;
        points.reserve(64);

        const auto addArc = [&](float centerX, float centerY, float startAngle, float endAngle, float radius, int segments) {
            if (radius <= 0.0f) {
                return;
            }
            for (int i = 0; i <= segments; ++i) {
                const float t = static_cast<float>(i) / static_cast<float>(segments);
                const float angle = startAngle + (endAngle - startAngle) * t;
                points.push_back({centerX + radius * std::cos(angle), centerY + radius * std::sin(angle)});
            }
        };

        points.push_back({x + tl, y});
        points.push_back({x + w - tr, y});
        addArc(x + w - tr, y + tr, -static_cast<float>(3.14159265358979323846) / 2.0f, 0.0f, tr, 8);
        points.push_back({x + w, y + h - br});
        addArc(x + w - br, y + h - br, 0.0f, static_cast<float>(3.14159265358979323846) / 2.0f, br, 8);
        points.push_back({x + bl, y + h});
        addArc(x + bl, y + h - bl, static_cast<float>(3.14159265358979323846) / 2.0f, static_cast<float>(3.14159265358979323846), bl, 8);
        points.push_back({x, y + tl});
        addArc(x + tl, y + tl, static_cast<float>(3.14159265358979323846), 3.0f * static_cast<float>(3.14159265358979323846) / 2.0f, tl, 8);

        if (points.size() < 3) {
            return;
        }

        for (size_t i = 1; i + 1 < points.size(); ++i) {
            rectVerts.push_back({points[0].x, points[0].y, r, g, b, a});
            rectVerts.push_back({points[i].x, points[i].y, r, g, b, a});
            rectVerts.push_back({points[i + 1].x, points[i + 1].y, r, g, b, a});
        }
    };

    for (const auto& r : cmd.rects) {
        const float baseAlpha = ((r.color >> 24) & 0xFF) / 255.0f;
        const float red = ((r.color >> 16) & 0xFF) / 255.0f;
        const float green = ((r.color >> 8) & 0xFF) / 255.0f;
        const float blue = (r.color & 0xFF) / 255.0f;
        const float alpha = baseAlpha * r.opacity;

        float clippedX = 0.0f;
        float clippedY = 0.0f;
        float clippedW = 0.0f;
        float clippedH = 0.0f;
        const bool clipped = ClipRectToBounds(r.x, r.y, r.w, r.h, r.clip, clippedX, clippedY, clippedW, clippedH);
        if (clipped) {
            if (clippedW > 0.0f && clippedH > 0.0f) {
                pushRect(clippedX, clippedY, clippedW, clippedH, red, green, blue, alpha);
            }
        } else {
            pushRect(r.x, r.y, r.w, r.h, red, green, blue, alpha);
        }
    }

    for (const auto& r : cmd.roundedRects) {
        const float baseAlpha = ((r.color >> 24) & 0xFF) / 255.0f;
        const float red = ((r.color >> 16) & 0xFF) / 255.0f;
        const float green = ((r.color >> 8) & 0xFF) / 255.0f;
        const float blue = (r.color & 0xFF) / 255.0f;
        const float alpha = baseAlpha * r.opacity;

        float clippedX = 0.0f;
        float clippedY = 0.0f;
        float clippedW = 0.0f;
        float clippedH = 0.0f;
        const bool clipped = ClipRectToBounds(r.x, r.y, r.w, r.h, r.clip, clippedX, clippedY, clippedW, clippedH);
        if (clipped) {
            if (clippedW > 0.0f && clippedH > 0.0f) {
                pushRoundedRect(clippedX, clippedY, clippedW, clippedH, r.topLeft, r.topRight, r.bottomRight, r.bottomLeft, red, green, blue, alpha);
            }
        } else {
            pushRoundedRect(r.x, r.y, r.w, r.h, r.topLeft, r.topRight, r.bottomRight, r.bottomLeft, red, green, blue, alpha);
        }
    }

    if (!rectVerts.empty() && g_vs && g_ps && g_inputLayout && g_vertexBuffer) {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (SUCCEEDED(g_context->Map(g_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            const float vp[2] = {static_cast<float>(width), static_cast<float>(height)};
            memcpy(mapped.pData, vp, sizeof(vp));
            g_context->Unmap(g_constantBuffer.Get(), 0);
        }

        const float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        g_context->OMSetBlendState(blendState.Get(), blendFactor, 0xffffffff);

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        g_context->IASetInputLayout(g_inputLayout.Get());
        g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        g_context->IASetVertexBuffers(0, 1, g_vertexBuffer.GetAddressOf(), &stride, &offset);
        g_context->VSSetShader(g_vs.Get(), nullptr, 0);
        g_context->PSSetShader(g_ps.Get(), nullptr, 0);
        g_context->VSSetConstantBuffers(0, 1, g_constantBuffer.GetAddressOf());

        size_t remainingVertices = rectVerts.size();
        size_t vertexOffset = 0;
        while (remainingVertices > 0) {
            size_t chunkVertices = std::min(remainingVertices, kMaxRectVertices);
            D3D11_MAPPED_SUBRESOURCE vmap = {};
            if (SUCCEEDED(g_context->Map(g_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vmap))) {
                memcpy(vmap.pData, rectVerts.data() + vertexOffset, chunkVertices * sizeof(Vertex));
                g_context->Unmap(g_vertexBuffer.Get(), 0);
            }
            g_context->Draw(static_cast<UINT>(chunkVertices), 0);
            remainingVertices -= chunkVertices;
            vertexOffset += chunkVertices;
        }
        g_context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
    }

    struct TextVertex { float x, y; float u, v; float r, g, b, a; };
    constexpr size_t kMaxTextVertices = kVertexBufferCapacity / sizeof(TextVertex);
    std::vector<TextVertex> textVerts;
    textVerts.reserve(std::min<size_t>(cmd.texturedQuads.size() * 6, kMaxTextVertices));
    std::vector<uint32_t> textQuadTextureIds;
    textQuadTextureIds.reserve(cmd.texturedQuads.size());

    auto pushTextQuad = [&](float x, float y, float w, float h, float u0, float v0, float u1, float v1, float opacity, uint32_t textureId) {
        textVerts.push_back({x, y, u0, v0, 1.0f, 1.0f, 1.0f, opacity});
        textVerts.push_back({x + w, y, u1, v0, 1.0f, 1.0f, 1.0f, opacity});
        textVerts.push_back({x + w, y + h, u1, v1, 1.0f, 1.0f, 1.0f, opacity});
        textVerts.push_back({x + w, y + h, u1, v1, 1.0f, 1.0f, 1.0f, opacity});
        textVerts.push_back({x, y + h, u0, v1, 1.0f, 1.0f, 1.0f, opacity});
        textVerts.push_back({x, y, u0, v0, 1.0f, 1.0f, 1.0f, opacity});
        textQuadTextureIds.push_back(textureId);
    };

    for (const auto& q : cmd.texturedQuads) {
        float clippedX = 0.0f;
        float clippedY = 0.0f;
        float clippedW = 0.0f;
        float clippedH = 0.0f;
        const bool clipped = ClipRectToBounds(q.x, q.y, q.w, q.h, q.clip, clippedX, clippedY, clippedW, clippedH);
        if (clipped) {
            if (clippedW > 0.0f && clippedH > 0.0f) {
                const float uSpan = q.u1 - q.u0;
                const float vSpan = q.v1 - q.v0;
                const float xDelta = clippedX - q.x;
                const float yDelta = clippedY - q.y;
                const float newU0 = q.u0 + (xDelta / q.w) * uSpan;
                const float newV0 = q.v0 + (yDelta / q.h) * vSpan;
                const float newU1 = newU0 + (clippedW / q.w) * uSpan;
                const float newV1 = newV0 + (clippedH / q.h) * vSpan;
                pushTextQuad(clippedX, clippedY, clippedW, clippedH, newU0, newV0, newU1, newV1, q.opacity, q.textureId);
            }
        } else {
            pushTextQuad(q.x, q.y, q.w, q.h, q.u0, q.v0, q.u1, q.v1, q.opacity, q.textureId);
        }
    }

    if (!textVerts.empty() && g_vsText && g_psText && g_inputLayoutText && g_vertexBuffer && g_samplerState) {
        const float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        g_context->OMSetBlendState(blendState.Get(), blendFactor, 0xffffffff);

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (SUCCEEDED(g_context->Map(g_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            const float vp[2] = {static_cast<float>(width), static_cast<float>(height)};
            memcpy(mapped.pData, vp, sizeof(vp));
            g_context->Unmap(g_constantBuffer.Get(), 0);
        }

        UINT stride = sizeof(TextVertex);
        UINT offset = 0;
        g_context->IASetInputLayout(g_inputLayoutText.Get());
        g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        g_context->IASetVertexBuffers(0, 1, g_vertexBuffer.GetAddressOf(), &stride, &offset);
        g_context->VSSetShader(g_vsText.Get(), nullptr, 0);
        g_context->PSSetShader(g_psText.Get(), nullptr, 0);
        g_context->VSSetConstantBuffers(0, 1, g_constantBuffer.GetAddressOf());
        g_context->PSSetSamplers(0, 1, g_samplerState.GetAddressOf());

        size_t remainingVertices = textVerts.size();
        size_t vertexOffset = 0;
        while (remainingVertices > 0) {
            size_t chunkVertices = std::min(remainingVertices, kMaxTextVertices);
            D3D11_MAPPED_SUBRESOURCE vmap = {};
            if (SUCCEEDED(g_context->Map(g_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vmap))) {
                memcpy(vmap.pData, textVerts.data() + vertexOffset, chunkVertices * sizeof(TextVertex));
                g_context->Unmap(g_vertexBuffer.Get(), 0);
            }

            size_t chunkQuads = chunkVertices / 6;
            size_t quadOffset = vertexOffset / 6;
            size_t quadsDrawn = 0;
            while (quadsDrawn < chunkQuads) {
                size_t qi = quadOffset + quadsDrawn;
                uint32_t texId = textQuadTextureIds[qi];
                size_t batchCount = 1;
                while (qi + batchCount < textQuadTextureIds.size() && batchCount < chunkQuads - quadsDrawn && textQuadTextureIds[qi + batchCount] == texId) {
                    ++batchCount;
                }

                ComPtr<ID3D11ShaderResourceView> srv = GetTextureShaderResourceView(texId);
                if (srv) {
                    g_context->PSSetShaderResources(0, 1, srv.GetAddressOf());
                    g_context->Draw(static_cast<UINT>(batchCount * 6), static_cast<UINT>((quadsDrawn * 6)));
                }

                quadsDrawn += batchCount;
            }

            remainingVertices -= chunkVertices;
            vertexOffset += chunkVertices;
        }

        g_context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
    }
}

}
