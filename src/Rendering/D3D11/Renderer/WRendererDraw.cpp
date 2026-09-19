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
#include <any>
#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>
#include <NeoWPF/Diagnostics/Logger.h>

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

    std::size_t textCommandCount = 0;
    for (const auto& command : cmd.commands) {
        if (command.kind == CommandList::CommandKind::TexturedQuad) {
            ++textCommandCount;
        }
    }
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextRendering,
        NeoWPF::Diagnostics::Level::Debug,
        "[D3D11] ExecuteRendererDraw commands=%zu texturedQuads=%zu target=%p",
        cmd.commands.size(), textCommandCount, rtv);

    const unsigned int width = GetCurrentRendererWidth();
    const unsigned int height = GetCurrentRendererHeight();

    g_context->OMSetRenderTargets(1, &rtv, nullptr);
    D3D11_VIEWPORT vp = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
    g_context->RSSetViewports(1, &vp);

    uint32_t bgColor = 0xFFFFFFFFu;
    if (cmd.hasClearColor) {
        bgColor = cmd.clearColor;
    } else {
        NeoWPF::UI::Color resourceColor;
        if (NeoWPF::Core::ApplicationResources::Resources::TryGetColor(
                "Window.BackgroundColor", resourceColor)) {
            bgColor = resourceColor.ToArgb();
        }
    }

    const float a = ((bgColor >> 24) & 0xFF) / 255.0f;
    const float r = ((bgColor >> 16) & 0xFF) / 255.0f;
    const float g = ((bgColor >> 8) & 0xFF) / 255.0f;
    const float b = (bgColor & 0xFF) / 255.0f;
    float clearColor[4] = { r, g, b, a };
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
    rectVerts.reserve(cmd.commands.size() * 2);

    struct Point2 { float x; float y; };
    const auto rotatePoint = [](float x, float y, float centerX, float centerY, float angleRadians) {
        const float cosA = std::cos(angleRadians);
        const float sinA = std::sin(angleRadians);
        const float dx = x - centerX;
        const float dy = y - centerY;
        return Point2{centerX + dx * cosA - dy * sinA, centerY + dx * sinA + dy * cosA};
    };

    auto pushRect = [&](float x, float y, float w, float h, float r, float g, float b, float a, float rotationRadians, float pivotX, float pivotY) {
        if (w <= 0.0f || h <= 0.0f) {
            return;
        }

        const float centerX = pivotX;
        const float centerY = pivotY;
        const auto emitVertex = [&](float px, float py) {
            rectVerts.push_back({px, py, r, g, b, a});
        };

        if (std::fabs(rotationRadians) < 1e-6f) {
            emitVertex(x, y);
            emitVertex(x + w, y);
            emitVertex(x + w, y + h);
            emitVertex(x + w, y + h);
            emitVertex(x, y + h);
            emitVertex(x, y);
            return;
        }

        const Point2 p1 = rotatePoint(x, y, centerX, centerY, rotationRadians);
        const Point2 p2 = rotatePoint(x + w, y, centerX, centerY, rotationRadians);
        const Point2 p3 = rotatePoint(x + w, y + h, centerX, centerY, rotationRadians);
        const Point2 p4 = rotatePoint(x, y + h, centerX, centerY, rotationRadians);

        emitVertex(p1.x, p1.y);
        emitVertex(p2.x, p2.y);
        emitVertex(p3.x, p3.y);
        emitVertex(p3.x, p3.y);
        emitVertex(p4.x, p4.y);
        emitVertex(p1.x, p1.y);
    };

    auto pushRoundedRect = [&](float x, float y, float w, float h,
        float topLeft, float topRight, float bottomRight, float bottomLeft,
        float r, float g, float b, float a, float rotationRadians, float pivotX, float pivotY) {
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

        const float centerX = pivotX;
        const float centerY = pivotY;
        std::vector<Point2> transformedPoints;
        transformedPoints.reserve(points.size());

        if (std::fabs(rotationRadians) < 1e-6f) {
            transformedPoints = points;
        } else {
            for (const auto& point : points) {
                transformedPoints.push_back(rotatePoint(point.x, point.y, centerX, centerY, rotationRadians));
            }
        }

        for (size_t i = 1; i + 1 < transformedPoints.size(); ++i) {
            rectVerts.push_back({transformedPoints[0].x, transformedPoints[0].y, r, g, b, a});
            rectVerts.push_back({transformedPoints[i].x, transformedPoints[i].y, r, g, b, a});
            rectVerts.push_back({transformedPoints[i + 1].x, transformedPoints[i + 1].y, r, g, b, a});
        }
    };

    struct TextVertex { float x, y; float u, v; float r, g, b, a; };
    constexpr size_t kMaxTextVertices = kVertexBufferCapacity / sizeof(TextVertex);
    std::vector<TextVertex> textVerts;
    std::vector<uint32_t> textQuadTextureIds;

    auto pushTextQuad = [&](float x, float y, float w, float h, float u0, float v0, float u1, float v1, uint32_t tintColor, float opacity, uint32_t textureId, float rotationRadians = 0.0f, float pivotX = 0.0f, float pivotY = 0.0f) {
        const float baseAlpha = ((tintColor >> 24) & 0xFF) / 255.0f;
        const float red = ((tintColor >> 16) & 0xFF) / 255.0f;
        const float green = ((tintColor >> 8) & 0xFF) / 255.0f;
        const float blue = (tintColor & 0xFF) / 255.0f;
        const float finalAlpha = baseAlpha * opacity;

        if (rotationRadians == 0.0f) {
            textVerts.push_back({x, y, u0, v0, red, green, blue, finalAlpha});
            textVerts.push_back({x + w, y, u1, v0, red, green, blue, finalAlpha});
            textVerts.push_back({x + w, y + h, u1, v1, red, green, blue, finalAlpha});
            textVerts.push_back({x + w, y + h, u1, v1, red, green, blue, finalAlpha});
            textVerts.push_back({x, y + h, u0, v1, red, green, blue, finalAlpha});
            textVerts.push_back({x, y, u0, v0, red, green, blue, finalAlpha});
            textQuadTextureIds.push_back(textureId);
            return;
        }

        const float cx = pivotX;
        const float cy = pivotY;
        const float cos_a = std::cos(rotationRadians);
        const float sin_a = std::sin(rotationRadians);

        auto rotPoint = [&](float px, float py) {
            const float dx = px - cx;
            const float dy = py - cy;
            const float rx = dx * cos_a - dy * sin_a + cx;
            const float ry = dx * sin_a + dy * cos_a + cy;
            return std::pair<float, float>(rx, ry);
        };

        auto p0 = rotPoint(x, y);
        auto p1 = rotPoint(x + w, y);
        auto p2 = rotPoint(x + w, y + h);
        auto p3 = rotPoint(x, y + h);

        textVerts.push_back({p0.first, p0.second, u0, v0, red, green, blue, finalAlpha});
        textVerts.push_back({p1.first, p1.second, u1, v0, red, green, blue, finalAlpha});
        textVerts.push_back({p2.first, p2.second, u1, v1, red, green, blue, finalAlpha});
        textVerts.push_back({p2.first, p2.second, u1, v1, red, green, blue, finalAlpha});
        textVerts.push_back({p3.first, p3.second, u0, v1, red, green, blue, finalAlpha});
        textVerts.push_back({p0.first, p0.second, u0, v0, red, green, blue, finalAlpha});
        textQuadTextureIds.push_back(textureId);
    };

    auto flushSolidBatch = [&]() {
        if (rectVerts.empty() || !g_vs || !g_ps || !g_inputLayout || !g_vertexBuffer) {
            return;
        }

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
        rectVerts.clear();
    };

    auto flushTextBatch = [&]() {
        if (textVerts.empty() || !g_vsText || !g_psText || !g_inputLayoutText || !g_vertexBuffer || !g_samplerState) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextRendering,
                NeoWPF::Diagnostics::Level::Debug,
                "[D3D11] text batch skipped vertices=%zu shaderReady=%d",
                textVerts.size(), g_vsText && g_psText && g_inputLayoutText && g_vertexBuffer && g_samplerState ? 1 : 0);
            return;
        }

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextRendering,
            NeoWPF::Diagnostics::Level::Debug,
            "[D3D11] flush text batch vertices=%zu quads=%zu textures=%zu",
            textVerts.size(), textVerts.size() / 6, textQuadTextureIds.size());

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
        textVerts.clear();
        textQuadTextureIds.clear();
    };

    CommandList::CommandKind lastKind = CommandList::CommandKind::Rect;
    bool hasPendingKind = false;

    for (const auto& command : cmd.commands) {
        if (hasPendingKind && command.kind != lastKind) {
            if (lastKind == CommandList::CommandKind::TexturedQuad) {
                flushTextBatch();
            } else {
                flushSolidBatch();
            }
        }

        switch (command.kind) {
        case CommandList::CommandKind::Rect: {
            const auto& r = command.rect;
            const float baseAlpha = ((r.color >> 24) & 0xFF) / 255.0f;
            const float red = ((r.color >> 16) & 0xFF) / 255.0f;
            const float green = ((r.color >> 8) & 0xFF) / 255.0f;
            const float blue = (r.color & 0xFF) / 255.0f;
            const float alpha = baseAlpha * r.opacity;

            if (std::fabs(r.rotationRadians) < 1e-6f) {
                double clippedX = 0.0;
                double clippedY = 0.0;
                double clippedW = 0.0;
                double clippedH = 0.0;
                const bool clipped = ClipRectToBounds(r.x, r.y, r.w, r.h, r.clip, clippedX, clippedY, clippedW, clippedH);
                if (clipped) {
                    if (clippedW > 0.0f && clippedH > 0.0f) {
                        pushRect(static_cast<float>(clippedX), static_cast<float>(clippedY), static_cast<float>(clippedW), static_cast<float>(clippedH), red, green, blue, alpha, r.rotationRadians, static_cast<float>(r.pivotX), static_cast<float>(r.pivotY));
                    }
                } else {
                    pushRect(static_cast<float>(r.x), static_cast<float>(r.y), static_cast<float>(r.w), static_cast<float>(r.h), red, green, blue, alpha, r.rotationRadians, static_cast<float>(r.pivotX), static_cast<float>(r.pivotY));
                }
            } else {
                pushRect(static_cast<float>(r.x), static_cast<float>(r.y), static_cast<float>(r.w), static_cast<float>(r.h), red, green, blue, alpha, r.rotationRadians, static_cast<float>(r.pivotX), static_cast<float>(r.pivotY));
            }
            lastKind = command.kind;
            hasPendingKind = true;
            break;
        }
        case CommandList::CommandKind::RoundedRect: {
            const auto& r = command.roundedRect;
            const float baseAlpha = ((r.color >> 24) & 0xFF) / 255.0f;
            const float red = ((r.color >> 16) & 0xFF) / 255.0f;
            const float green = ((r.color >> 8) & 0xFF) / 255.0f;
            const float blue = (r.color & 0xFF) / 255.0f;
            const float alpha = baseAlpha * r.opacity;

            if (std::fabs(r.rotationRadians) < 1e-6f) {
                double clippedX = 0.0;
                double clippedY = 0.0;
                double clippedW = 0.0;
                double clippedH = 0.0;
                const bool clipped = ClipRectToBounds(r.x, r.y, r.w, r.h, r.clip, clippedX, clippedY, clippedW, clippedH);
                if (clipped) {
                    if (clippedW > 0.0f && clippedH > 0.0f) {
                        pushRoundedRect(static_cast<float>(clippedX), static_cast<float>(clippedY), static_cast<float>(clippedW), static_cast<float>(clippedH), static_cast<float>(r.topLeft), static_cast<float>(r.topRight), static_cast<float>(r.bottomRight), static_cast<float>(r.bottomLeft), red, green, blue, alpha, r.rotationRadians, static_cast<float>(r.pivotX), static_cast<float>(r.pivotY));
                    }
                } else {
                    pushRoundedRect(static_cast<float>(r.x), static_cast<float>(r.y), static_cast<float>(r.w), static_cast<float>(r.h), static_cast<float>(r.topLeft), static_cast<float>(r.topRight), static_cast<float>(r.bottomRight), static_cast<float>(r.bottomLeft), red, green, blue, alpha, r.rotationRadians, static_cast<float>(r.pivotX), static_cast<float>(r.pivotY));
                }
            } else {
                pushRoundedRect(static_cast<float>(r.x), static_cast<float>(r.y), static_cast<float>(r.w), static_cast<float>(r.h), static_cast<float>(r.topLeft), static_cast<float>(r.topRight), static_cast<float>(r.bottomRight), static_cast<float>(r.bottomLeft), red, green, blue, alpha, r.rotationRadians, static_cast<float>(r.pivotX), static_cast<float>(r.pivotY));
            }
            lastKind = command.kind;
            hasPendingKind = true;
            break;
        }
        case CommandList::CommandKind::TexturedQuad: {
            const auto& q = command.texturedQuad;
            if (hasPendingKind && lastKind != CommandList::CommandKind::TexturedQuad) {
                flushSolidBatch();
            }
            double clippedX = 0.0;
            double clippedY = 0.0;
            double clippedW = 0.0;
            double clippedH = 0.0;
            if (q.rotationRadians == 0.0f) {
                const bool clipped = ClipRectToBounds(q.x, q.y, q.w, q.h, q.clip, clippedX, clippedY, clippedW, clippedH);
                if (clipped) {
                    if (clippedW > 0.0f && clippedH > 0.0f) {
                        const float uSpan = q.u1 - q.u0;
                        const float vSpan = q.v1 - q.v0;
                        const double xDelta = clippedX - q.x;
                        const double yDelta = clippedY - q.y;
                        const float newU0 = static_cast<float>(q.u0 + (xDelta / q.w) * uSpan);
                        const float newV0 = static_cast<float>(q.v0 + (yDelta / q.h) * vSpan);
                        const float newU1 = static_cast<float>(newU0 + (clippedW / q.w) * uSpan);
                        const float newV1 = static_cast<float>(newV0 + (clippedH / q.h) * vSpan);
                        pushTextQuad(static_cast<float>(clippedX), static_cast<float>(clippedY), static_cast<float>(clippedW), static_cast<float>(clippedH), newU0, newV0, newU1, newV1, q.tintColor, q.opacity, q.textureId);
                    }
                } else {
                    pushTextQuad(static_cast<float>(q.x), static_cast<float>(q.y), static_cast<float>(q.w), static_cast<float>(q.h), q.u0, q.v0, q.u1, q.v1, q.tintColor, q.opacity, q.textureId);
                }
            } else {
                pushTextQuad(static_cast<float>(q.x), static_cast<float>(q.y), static_cast<float>(q.w), static_cast<float>(q.h), q.u0, q.v0, q.u1, q.v1, q.tintColor, q.opacity, q.textureId, q.rotationRadians, static_cast<float>(q.pivotX), static_cast<float>(q.pivotY));
            }
            lastKind = command.kind;
            hasPendingKind = true;
            break;
        }
        }
    }

    if (hasPendingKind) {
        if (lastKind == CommandList::CommandKind::TexturedQuad) {
            flushTextBatch();
        } else {
            flushSolidBatch();
        }
    }
}

}
