#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/Geometry/ArcTessellation.h>
#include <NeoWPF/Rendering/Geometry/PrimitiveTessellation.h>
<<<<<<< HEAD
=======
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
>>>>>>> 32fba71e27e8d22835fe2c345795c72f32463c81
#include <NeoWPF/Rendering/Geometry/Clipping.h>
#include <d3d11.h>
#include <wrl.h>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <Windows.h>
#include <limits>
#include <NeoWPF/Diagnostics/Logger.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

using Microsoft::WRL::ComPtr;
using namespace NeoWPF::Rendering;

/* @note
/  Do know that I will not be changing any of the functionality in here
/  related to globals unless it literally stops the app from rendering
*/

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
    unsigned int GetRendererWidth(HWND hwnd);
    unsigned int GetRendererHeight(HWND hwnd);
    ID3D11RenderTargetView* GetRendererTargetView(HWND hwnd);
    bool IsRendererTransparent(HWND hwnd);
    ComPtr<ID3D11ShaderResourceView> GetTextureShaderResourceView(uint32_t id);
}

namespace NeoWPF::Rendering::D3D11 {

void ExecuteRendererDraw(HWND hwnd, const CommandList& cmd)
{
    ID3D11RenderTargetView* rtv = GetRendererTargetView(hwnd);
    static bool loggedMissingTarget = false;
    if (!rtv) {
        if (!loggedMissingTarget) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
                NeoWPF::Diagnostics::Level::Info,
                "[D3D11] ExecuteRendererDraw skipped: render target is unavailable");
            loggedMissingTarget = true;
        }
        return;
    }

    std::size_t textCommandCount = 0;
    for (const auto& command : cmd.commands) {
        if (command.GetKind() == CommandList::CommandKind::TexturedQuad) {
            ++textCommandCount;
        }
    }
    std::size_t roundedRectCount = 0;
    std::size_t arcCount = 0;
    std::size_t rectCount = 0;
    for (const auto& command : cmd.commands) {
        if (command.GetKind() == CommandList::CommandKind::RoundedRect) {
            ++roundedRectCount;
        } else if (command.GetKind() == CommandList::CommandKind::Arc) {
            ++arcCount;
        } else if (command.GetKind() == CommandList::CommandKind::Rect) {
            ++rectCount;
        }
    }
    static std::size_t lastCommandCount = std::numeric_limits<std::size_t>::max();
    static std::size_t lastRoundedRectCount = std::numeric_limits<std::size_t>::max();
    static std::size_t lastArcCount = std::numeric_limits<std::size_t>::max();
    static std::size_t lastTextCommandCount = std::numeric_limits<std::size_t>::max();
    const bool commandShapeChanged = cmd.commands.size() != lastCommandCount ||
        roundedRectCount != lastRoundedRectCount ||
        arcCount != lastArcCount ||
        textCommandCount != lastTextCommandCount;
    if (commandShapeChanged) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Debug,
            "[D3D11] Command shape changed commands=%zu rects=%zu roundedRects=%zu arcs=%zu texturedQuads=%zu target=%p",
            cmd.commands.size(), rectCount, roundedRectCount, arcCount, textCommandCount, rtv);
        lastCommandCount = cmd.commands.size();
        lastRoundedRectCount = roundedRectCount;
        lastArcCount = arcCount;
        lastTextCommandCount = textCommandCount;
        for (std::size_t index = 0; index < cmd.commands.size(); ++index) {
            const auto& command = cmd.commands[index];
            if (command.GetKind() != CommandList::CommandKind::RoundedRect) {
                continue;
            }
            const auto& rounded = command.GetRoundedRect();
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
                NeoWPF::Diagnostics::Level::Trace,
                "[D3D11] RoundedRect index=%zu rect=%.1f,%.1fx%.1f,%.1f color=0x%08X opacity=%.2f",
                index, rounded.x, rounded.y, rounded.w, rounded.h,
                rounded.paint.color, rounded.opacity);
        }
    }

    const unsigned int width = GetRendererWidth(hwnd);
    const unsigned int height = GetRendererHeight(hwnd);

    g_context->OMSetRenderTargets(1, &rtv, nullptr);
    D3D11_VIEWPORT vp = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
    g_context->RSSetViewports(1, &vp);

    const uint32_t bgColor = cmd.hasClearColor ? cmd.clearColor : 0xFFFFFFFFu;

    const float a = ((bgColor >> 24) & 0xFF) / 255.0f;
    float r = ((bgColor >> 16) & 0xFF) / 255.0f;
    float g = ((bgColor >> 8) & 0xFF) / 255.0f;
    float b = (bgColor & 0xFF) / 255.0f;
    if (IsRendererTransparent(hwnd)) {
        r *= a;
        g *= a;
        b *= a;
    }
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
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        const auto result = g_device->CreateBlendState(&blendDesc, &blendState);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            result == S_OK ? NeoWPF::Diagnostics::Level::Info : NeoWPF::Diagnostics::Level::Info,
            "[D3D11] Solid blend state creation result=0x%08X ready=%d",
            static_cast<unsigned int>(result), blendState ? 1 : 0);
    }

    struct Vertex { float x, y; float r, g, b, a; };
    constexpr size_t kVertexBufferCapacity = 1 << 20;
    constexpr size_t kMaxSolidVertices = (kVertexBufferCapacity / sizeof(Vertex) / 3) * 3;
    std::vector<Vertex> rectVerts;
    std::vector<GeometryVertex> geometry;
    rectVerts.reserve(cmd.commands.size() * 2);

    const auto pushRect = [&](double x, double y, double w, double h,
        float red, float green, float blue, float alpha, float rotationRadians,
        double pivotX, double pivotY, const ClipRect& clip) {
        TessellateRect(geometry,
            x, y, w, h, clip, rotationRadians, pivotX, pivotY);
        for (const auto& vertex : geometry) {
            rectVerts.push_back({static_cast<float>(vertex.x), static_cast<float>(vertex.y),
                                 red, green, blue, alpha});
        }
    };

    const auto pushRoundedRect = [&](double x, double y, double w, double h,
        double topLeft, double topRight, double bottomRight, double bottomLeft,
        float red, float green, float blue, float alpha, float rotationRadians,
        double pivotX, double pivotY, const ClipRect& clip) {
        TessellateRoundedRect(geometry,
            x, y, w, h, topLeft, topRight, bottomRight, bottomLeft,
            clip, rotationRadians, pivotX, pivotY);
        for (const auto& vertex : geometry) {
            rectVerts.push_back({static_cast<float>(vertex.x), static_cast<float>(vertex.y),
                                 red, green, blue, alpha});
        }
    };

    auto pushArc = [&](const CommandList::ArcData& arc) {
        const float baseAlpha = ((arc.paint.color >> 24) & 0xFF) / 255.0f;
        const float red = ((arc.paint.color >> 16) & 0xFF) / 255.0f;
        const float green = ((arc.paint.color >> 8) & 0xFF) / 255.0f;
        const float blue = (arc.paint.color & 0xFF) / 255.0f;
        TessellateAnnularArc(geometry,
            arc.centerX,
            arc.centerY,
            arc.radius,
            arc.thickness,
            arc.startAngleRadians,
            arc.sweepAngleRadians,
            arc.clip);
        for (const auto& vertex : geometry) {
            rectVerts.push_back({
                static_cast<float>(vertex.x),
                static_cast<float>(vertex.y),
                red,
                green,
                blue,
                baseAlpha * arc.opacity,
            });
        }
    };

    struct TextVertex { float x, y; float u, v; float r, g, b, a; };
    constexpr size_t kMaxTextVertices = (kVertexBufferCapacity / sizeof(TextVertex) / 3) * 3;
    std::vector<TextVertex> textVerts;
    struct TextDrawRange { uint32_t textureId; size_t firstVertex; size_t vertexCount; };
    std::vector<TextDrawRange> textDrawRanges;

    auto pushTextQuad = [&](double x, double y, double w, double h,
        float u0, float v0, float u1, float v1, uint32_t tintColor, float opacity,
        uint32_t textureId, const ClipRect& clip, float rotationRadians,
        double pivotX, double pivotY) {
        const float baseAlpha = ((tintColor >> 24) & 0xFF) / 255.0f;
        const float red = ((tintColor >> 16) & 0xFF) / 255.0f;
        const float green = ((tintColor >> 8) & 0xFF) / 255.0f;
        const float blue = (tintColor & 0xFF) / 255.0f;
        const float finalAlpha = baseAlpha * opacity;
        TessellateRect(geometry,
            x, y, w, h, clip, rotationRadians, pivotX, pivotY, u0, v0, u1, v1);
        const size_t firstVertex = textVerts.size();
        for (const auto& vertex : geometry) {
            textVerts.push_back({static_cast<float>(vertex.x), static_cast<float>(vertex.y),
                                 vertex.u, vertex.v, red, green, blue, finalAlpha});
        }
        if (textVerts.size() > firstVertex) {
            textDrawRanges.push_back({textureId, firstVertex, textVerts.size() - firstVertex});
        }
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
            size_t chunkVertices = std::min(remainingVertices, kMaxSolidVertices);
            D3D11_MAPPED_SUBRESOURCE vmap = {};
            if (FAILED(g_context->Map(g_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vmap))) {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
                    NeoWPF::Diagnostics::Level::Debug,
                    "[D3D11] Failed to map the solid vertex stream");
                remainingVertices -= chunkVertices;
                vertexOffset += chunkVertices;
                continue;
            }
            memcpy(vmap.pData, rectVerts.data() + vertexOffset, chunkVertices * sizeof(Vertex));
            g_context->Unmap(g_vertexBuffer.Get(), 0);
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
            textVerts.size(), textDrawRanges.size(), textDrawRanges.size());

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
            if (FAILED(g_context->Map(g_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vmap))) {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
                    NeoWPF::Diagnostics::Level::Debug,
                    "[D3D11] Failed to map the textured vertex stream");
                remainingVertices -= chunkVertices;
                vertexOffset += chunkVertices;
                continue;
            }
            memcpy(vmap.pData, textVerts.data() + vertexOffset, chunkVertices * sizeof(TextVertex));
            g_context->Unmap(g_vertexBuffer.Get(), 0);

            size_t rangeIndex = 0;
            const size_t chunkEnd = vertexOffset + chunkVertices;
            while (rangeIndex < textDrawRanges.size()) {
                const auto& range = textDrawRanges[rangeIndex];
                const size_t rangeEnd = range.firstVertex + range.vertexCount;
                if (rangeEnd <= vertexOffset) {
                    ++rangeIndex;
                    continue;
                }
                if (range.firstVertex >= chunkEnd) {
                    break;
                }

                const uint32_t textureId = range.textureId;
                const size_t drawStart = (std::max)(range.firstVertex, vertexOffset);
                size_t drawEnd = (std::min)(rangeEnd, chunkEnd);
                size_t nextRange = rangeIndex + 1;
                while (nextRange < textDrawRanges.size() &&
                       textDrawRanges[nextRange].textureId == textureId &&
                       textDrawRanges[nextRange].firstVertex == drawEnd && drawEnd < chunkEnd) {
                    drawEnd = (std::min)(
                        textDrawRanges[nextRange].firstVertex + textDrawRanges[nextRange].vertexCount,
                        chunkEnd);
                    ++nextRange;
                }

                ComPtr<ID3D11ShaderResourceView> srv = GetTextureShaderResourceView(textureId);
                if (srv) {
                    g_context->PSSetShaderResources(0, 1, srv.GetAddressOf());
                    g_context->Draw(static_cast<UINT>(drawEnd - drawStart),
                                    static_cast<UINT>(drawStart - vertexOffset));
                }
                rangeIndex = nextRange;
            }

            remainingVertices -= chunkVertices;
            vertexOffset += chunkVertices;
        }

        g_context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
        textVerts.clear();
        textDrawRanges.clear();
    };

    CommandList::CommandKind lastKind = CommandList::CommandKind::Rect;
    bool hasPendingKind = false;

    for (const auto& command : cmd.commands) {
        const bool isTextured = command.GetKind() == CommandList::CommandKind::TexturedQuad;
        const bool wasTextured = lastKind == CommandList::CommandKind::TexturedQuad;
        if (hasPendingKind && isTextured != wasTextured) {
            if (wasTextured) {
                flushTextBatch();
            } else {
                flushSolidBatch();
            }
        }

        switch (command.GetKind()) {
        case CommandList::CommandKind::Rect: {
            const auto& r = command.GetRect();
            const float baseAlpha = ((r.paint.color >> 24) & 0xFF) / 255.0f;
            const float red = ((r.paint.color >> 16) & 0xFF) / 255.0f;
            const float green = ((r.paint.color >> 8) & 0xFF) / 255.0f;
            const float blue = (r.paint.color & 0xFF) / 255.0f;
            const float alpha = baseAlpha * r.opacity;

            pushRect(r.x, r.y, r.w, r.h, red, green, blue, alpha,
                     r.rotationRadians, r.pivotX, r.pivotY, r.clip);
            lastKind = command.GetKind();
            hasPendingKind = true;
            break;
        }
        case CommandList::CommandKind::RoundedRect: {
            const auto& r = command.GetRoundedRect();
            const float baseAlpha = ((r.paint.color >> 24) & 0xFF) / 255.0f;
            const float red = ((r.paint.color >> 16) & 0xFF) / 255.0f;
            const float green = ((r.paint.color >> 8) & 0xFF) / 255.0f;
            const float blue = (r.paint.color & 0xFF) / 255.0f;
            const float alpha = baseAlpha * r.opacity;

            pushRoundedRect(r.x, r.y, r.w, r.h, r.topLeft, r.topRight,
                            r.bottomRight, r.bottomLeft, red, green, blue, alpha,
                            r.rotationRadians, r.pivotX, r.pivotY, r.clip);
            lastKind = command.GetKind();
            hasPendingKind = true;
            break;
        }
        case CommandList::CommandKind::Arc: {
            pushArc(command.GetArc());
            lastKind = command.GetKind();
            hasPendingKind = true;
            break;
        }
        case CommandList::CommandKind::TexturedQuad: {
            const auto& q = command.GetTexturedQuad();
            pushTextQuad(q.x, q.y, q.w, q.h, q.u0, q.v0, q.u1, q.v1,
                         q.tint.color, q.opacity, q.textureId, q.clip,
                         q.rotationRadians, q.pivotX, q.pivotY);
            lastKind = command.GetKind();
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
