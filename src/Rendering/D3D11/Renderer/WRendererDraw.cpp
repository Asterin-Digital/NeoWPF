#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/Geometry/ArcTessellation.h>
#include <NeoWPF/Rendering/Geometry/PrimitiveTessellation.h>
#include <NeoWPF/Rendering/Geometry/Clipping.h>
#include <NeoWPF/Rendering/Geometry/DrawBatcher.h>
#include <NeoWPF/Rendering/Paint/RampBaker.h>
#include <d3d11.h>
#include <wrl.h>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cmath>
#include <windows.h>
#include <limits>
#include <span>
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
    std::uint64_t g_gradientTextureHash = 0;
    std::uint32_t g_gradientTextureId = 0;

    extern ComPtr<ID3D11Device> g_device;
    extern ComPtr<ID3D11DeviceContext> g_context;
    extern ComPtr<ID3D11VertexShader> g_vs;
    extern ComPtr<ID3D11PixelShader> g_ps;
    extern ComPtr<ID3D11InputLayout> g_inputLayout;
    extern ComPtr<ID3D11VertexShader> g_vsText;
    extern ComPtr<ID3D11PixelShader> g_psText;
    extern ComPtr<ID3D11InputLayout> g_inputLayoutText;
    extern ComPtr<ID3D11VertexShader> g_vsGradient;
    extern ComPtr<ID3D11PixelShader> g_psGradient;
    extern ComPtr<ID3D11InputLayout> g_inputLayoutGradient;
    extern ComPtr<ID3D11SamplerState> g_samplerState;
    extern ComPtr<ID3D11Buffer> g_vertexBuffer;
    extern ComPtr<ID3D11Buffer> g_constantBuffer;
    extern ComPtr<ID3D11RasterizerState> g_rasterizerState;
    extern ComPtr<ID3D11DepthStencilState> g_depthStencilState;
    unsigned int GetRendererWidth(HWND hwnd);
    unsigned int GetRendererHeight(HWND hwnd);
    ID3D11RenderTargetView* GetRendererTargetView(HWND hwnd);
    bool IsRendererTransparent(HWND hwnd);
    ComPtr<ID3D11ShaderResourceView> GetTextureShaderResourceView(uint32_t id);
    uint32_t CreateTexture(unsigned int width, unsigned int height, const void* rgbaData);
    void DestroyTexture(uint32_t id);

    void ResetGradientAtlas() noexcept
    {
        g_gradientTextureHash = 0;
        g_gradientTextureId = 0;
    }
}

namespace NeoWPF::Rendering::D3D11 {

constexpr bool kUseExplicitRasterizerState = false;
constexpr bool kUseExplicitDepthStencilState = false;
constexpr bool kClearTextShaderBindings = false;

void ExecuteRendererDraw(HWND hwnd, const CommandList& cmd, double scaleFactor)
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
    const auto batches = BuildDrawBatches(cmd);
    std::uint64_t batchShapeHash = 14695981039346656037ull;
    const auto mixBatchShapeValue = [&batchShapeHash](std::uint64_t value) {
        batchShapeHash ^= value;
        batchShapeHash *= 1099511628211ull;
    };
    mixBatchShapeValue(batches.size());
    for (const auto& batch : batches) {
        mixBatchShapeValue(static_cast<std::uint64_t>(batch.kind));
        mixBatchShapeValue(batch.firstCommand);
        mixBatchShapeValue(batch.commandCount);
        mixBatchShapeValue(batch.textureId);
    }
    static std::size_t lastCommandCount = std::numeric_limits<std::size_t>::max();
    static std::size_t lastRoundedRectCount = std::numeric_limits<std::size_t>::max();
    static std::size_t lastArcCount = std::numeric_limits<std::size_t>::max();
    static std::size_t lastTextCommandCount = std::numeric_limits<std::size_t>::max();
    static std::uint64_t lastBatchShapeHash = std::numeric_limits<std::uint64_t>::max();
    const bool commandShapeChanged = cmd.commands.size() != lastCommandCount ||
        roundedRectCount != lastRoundedRectCount ||
        arcCount != lastArcCount ||
        textCommandCount != lastTextCommandCount ||
        batchShapeHash != lastBatchShapeHash;
    if (commandShapeChanged) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Debug,
            "[D3D11] Command shape changed commands=%zu rects=%zu roundedRects=%zu arcs=%zu texturedQuads=%zu target=%p",
            cmd.commands.size(), rectCount, roundedRectCount, arcCount, textCommandCount, rtv);
        lastCommandCount = cmd.commands.size();
        lastRoundedRectCount = roundedRectCount;
        lastArcCount = arcCount;
        lastTextCommandCount = textCommandCount;
        lastBatchShapeHash = batchShapeHash;
        for (const auto& batch : batches) {
            NeoWPF::Diagnostics::Log(
                NeoWPF::Diagnostics::Category::Renderer,
                NeoWPF::Diagnostics::Level::Debug,
                "[D3D11] batch kind=%u firstCommand=%zu commandCount=%zu textureId=%u",
                static_cast<unsigned int>(batch.kind),
                batch.firstCommand, batch.commandCount, batch.textureId);
        }
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

    if constexpr (kUseExplicitRasterizerState) {
        if (!g_rasterizerState) {
            D3D11_RASTERIZER_DESC rasterizerDesc = {};
            rasterizerDesc.CullMode = D3D11_CULL_NONE;
            rasterizerDesc.FillMode = D3D11_FILL_SOLID;
            rasterizerDesc.DepthClipEnable = TRUE;
            rasterizerDesc.ScissorEnable = FALSE;
            const HRESULT hr = g_device->CreateRasterizerState(
                &rasterizerDesc, &g_rasterizerState);
            if (FAILED(hr)) {
                NeoWPF::Diagnostics::Catastrophic(
                    "[D3D11] CreateRasterizerState failed with HRESULT 0x%08X",
                    static_cast<unsigned int>(hr));
            }
        }
        if (g_rasterizerState) {
            g_context->RSSetState(g_rasterizerState.Get());
        }
    }

    if constexpr (kUseExplicitDepthStencilState) {
        if (!g_depthStencilState) {
            D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
            depthStencilDesc.DepthEnable = FALSE;
            depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
            depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
            depthStencilDesc.StencilEnable = FALSE;
            const HRESULT hr = g_device->CreateDepthStencilState(
                &depthStencilDesc, &g_depthStencilState);
            if (FAILED(hr)) {
                NeoWPF::Diagnostics::Catastrophic(
                    "[D3D11] CreateDepthStencilState failed with HRESULT 0x%08X",
                    static_cast<unsigned int>(hr));
            }
        }
        if (g_depthStencilState) {
            g_context->OMSetDepthStencilState(g_depthStencilState.Get(), 0);
        }
    }

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
    struct GradientVertex { float x, y; float worldX, worldY; float opacity; };
    constexpr size_t kVertexBufferCapacity = 1 << 20;
    constexpr size_t kMaxSolidVertices = (kVertexBufferCapacity / sizeof(Vertex) / 3) * 3;
    std::vector<Vertex> rectVerts;
    std::vector<GradientVertex> gradientVerts;
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
            const float vp[2] = {
                static_cast<float>(static_cast<double>(width) / scaleFactor),
                static_cast<float>(static_cast<double>(height) / scaleFactor)};
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
            const float vp[2] = {
                static_cast<float>(static_cast<double>(width) / scaleFactor),
                static_cast<float>(static_cast<double>(height) / scaleFactor)};
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
        if constexpr (kClearTextShaderBindings) {
            ID3D11ShaderResourceView* nullSrv = nullptr;
            ID3D11SamplerState* nullSampler = nullptr;
            g_context->PSSetShaderResources(0, 1, &nullSrv);
            g_context->PSSetSamplers(0, 1, &nullSampler);
        }
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
                    NeoWPF::Diagnostics::Log(
                        NeoWPF::Diagnostics::Category::TextRendering,
                        NeoWPF::Diagnostics::Level::Debug,
                        "[D3D11] text Draw vertices=%zu startVertex=%zu textureId=%u",
                        drawEnd - drawStart, drawStart - vertexOffset, textureId);
                    g_context->Draw(static_cast<UINT>(drawEnd - drawStart),
                                    static_cast<UINT>(drawStart - vertexOffset));
                } else {
                    NeoWPF::Diagnostics::Log(
                        NeoWPF::Diagnostics::Category::TextRendering,
                        NeoWPF::Diagnostics::Level::Debug,
                        "[D3D11] text batch missing shader resource view textureId=%u",
                        textureId);
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

    auto flushGradientBatch = [&](const DrawBatch& batch) {
        if (gradientVerts.empty() ||
            batch.gradientIndex >= cmd.gradients.size() ||
            !g_vsGradient || !g_psGradient || !g_inputLayoutGradient ||
            !g_vertexBuffer || !g_samplerState) {
            gradientVerts.clear();
            return;
        }

        const std::uint64_t nextGradientTextureHash = ComputeGradientAtlasHash(
            cmd.gradients, cmd.gradientStops);
        if (g_gradientTextureId == 0 ||
            g_gradientTextureHash != nextGradientTextureHash) {
            constexpr std::uint32_t kRampWidth = 256;
            constexpr std::uint32_t kRampHeight = 256;
            std::vector<std::uint8_t> rgba(kRampWidth * kRampHeight * 4, 0);
            std::vector<std::uint32_t> ramp(kRampWidth);
            for (std::size_t gradientIndex = 0;
                 gradientIndex < cmd.gradients.size() &&
                 gradientIndex < kRampHeight;
                 ++gradientIndex) {
                const auto& gradient = cmd.gradients[gradientIndex];
                const std::size_t stopOffset = gradient.stopOffset;
                const std::size_t count = std::min(
                    static_cast<std::size_t>(gradient.stopCount),
                    cmd.gradientStops.size() > stopOffset
                        ? cmd.gradientStops.size() - stopOffset
                        : 0u);
                if (count == 0) {
                    continue;
                }
                BakeRamp(std::span<const GradientStopDesc>(
                             cmd.gradientStops.data() + stopOffset, count),
                         ramp, kRampWidth);
                for (std::size_t pixelIndex = 0; pixelIndex < kRampWidth; ++pixelIndex) {
                    const std::uint32_t argb = ramp[pixelIndex];
                    const std::size_t rgbaIndex =
                        (gradientIndex * kRampWidth + pixelIndex) * 4;
                    rgba[rgbaIndex + 0] = static_cast<std::uint8_t>((argb >> 16) & 0xFFu);
                    rgba[rgbaIndex + 1] = static_cast<std::uint8_t>((argb >> 8) & 0xFFu);
                    rgba[rgbaIndex + 2] = static_cast<std::uint8_t>(argb & 0xFFu);
                    rgba[rgbaIndex + 3] = static_cast<std::uint8_t>((argb >> 24) & 0xFFu);
                }
            }
            if (g_gradientTextureId != 0) {
                DestroyTexture(g_gradientTextureId);
            }
            g_gradientTextureId = CreateTexture(kRampWidth, kRampHeight, rgba.data());
            g_gradientTextureHash =
                g_gradientTextureId != 0 ? nextGradientTextureHash : 0;
        }
        const auto srv = GetTextureShaderResourceView(g_gradientTextureId);
        if (!srv) {
            gradientVerts.clear();
            return;
        }

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (FAILED(g_context->Map(g_constantBuffer.Get(), 0,
                                  D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            gradientVerts.clear();
            return;
        }
        const auto& gradient = cmd.gradients[batch.gradientIndex];
        struct GradientConstants {
            float viewport[2];
            float padding[2];
            float mapping0[4];
            float mapping1[4];
            float gradient[4];
        } constants{};
        static_assert(sizeof(GradientConstants) == 64);
        static_assert(offsetof(GradientConstants, mapping0) == 16);
        constants.viewport[0] = static_cast<float>(
            static_cast<double>(width) / scaleFactor);
        constants.viewport[1] = static_cast<float>(
            static_cast<double>(height) / scaleFactor);
        constants.mapping0[0] = static_cast<float>(gradient.mapping[0]);
        constants.mapping0[1] = static_cast<float>(gradient.mapping[1]);
        constants.mapping0[2] = static_cast<float>(gradient.mapping[2]);
        constants.mapping0[3] = static_cast<float>(gradient.mapping[3]);
        constants.mapping1[0] = static_cast<float>(gradient.mapping[4]);
        constants.mapping1[1] = static_cast<float>(gradient.mapping[5]);
        constants.mapping1[2] = static_cast<float>(gradient.focalX);
        constants.mapping1[3] = static_cast<float>(gradient.focalY);
        constants.gradient[0] =
            batch.kind == DrawBatchKind::LinearGradient ? 0.0f : 1.0f;
        constants.gradient[1] = gradient.spread == GradientSpreadMethod::Pad
            ? 0.0f
            : gradient.spread == GradientSpreadMethod::Reflect
                ? 1.0f
                : 2.0f;
        constants.gradient[2] =
            (static_cast<float>(batch.gradientIndex) + 0.5f) / 256.0f;
        constants.gradient[3] = gradient.opacity;
        memcpy(mapped.pData, &constants, sizeof(constants));
        g_context->Unmap(g_constantBuffer.Get(), 0);

        const float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        g_context->OMSetBlendState(blendState.Get(), blendFactor, 0xffffffff);
        UINT stride = sizeof(GradientVertex);
        UINT offset = 0;
        g_context->IASetInputLayout(g_inputLayoutGradient.Get());
        g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        g_context->IASetVertexBuffers(0, 1, g_vertexBuffer.GetAddressOf(), &stride, &offset);
        g_context->VSSetShader(g_vsGradient.Get(), nullptr, 0);
        g_context->PSSetShader(g_psGradient.Get(), nullptr, 0);
        g_context->VSSetConstantBuffers(0, 1, g_constantBuffer.GetAddressOf());
        g_context->PSSetConstantBuffers(0, 1, g_constantBuffer.GetAddressOf());
        g_context->PSSetShaderResources(0, 1, srv.GetAddressOf());
        g_context->PSSetSamplers(0, 1, g_samplerState.GetAddressOf());

        std::size_t remainingVertices = gradientVerts.size();
        std::size_t vertexOffset = 0;
        constexpr std::size_t kMaxGradientVertices =
            (kVertexBufferCapacity / sizeof(GradientVertex) / 3) * 3;
        while (remainingVertices > 0) {
            const std::size_t chunkVertices =
                (std::min)(remainingVertices, kMaxGradientVertices);
            D3D11_MAPPED_SUBRESOURCE vertexMap = {};
            if (SUCCEEDED(g_context->Map(g_vertexBuffer.Get(), 0,
                                         D3D11_MAP_WRITE_DISCARD, 0, &vertexMap))) {
                memcpy(vertexMap.pData, gradientVerts.data() + vertexOffset,
                       chunkVertices * sizeof(GradientVertex));
                g_context->Unmap(g_vertexBuffer.Get(), 0);
                g_context->Draw(static_cast<UINT>(chunkVertices), 0);
            }
            remainingVertices -= chunkVertices;
            vertexOffset += chunkVertices;
        }
        g_context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
        gradientVerts.clear();
    };

    for (const auto& batch : batches) {
        if (batch.kind == DrawBatchKind::Textured) {
            flushSolidBatch();
            for (std::size_t index = batch.firstCommand;
                 index < batch.firstCommand + batch.commandCount;
                 ++index) {
                const auto& q = cmd.commands[index].GetTexturedQuad();
                pushTextQuad(q.x, q.y, q.w, q.h, q.u0, q.v0, q.u1, q.v1,
                             q.tint.color, q.opacity, q.textureId, q.clip,
                             q.rotationRadians, q.pivotX, q.pivotY);
            }
            flushTextBatch();
            continue;
        }

        flushTextBatch();
        if (batch.kind == DrawBatchKind::Solid) {
            for (std::size_t index = batch.firstCommand;
                 index < batch.firstCommand + batch.commandCount;
                 ++index) {
                const auto& command = cmd.commands[index];
                switch (command.GetKind()) {
                case CommandList::CommandKind::Rect: {
                    const auto& r = command.GetRect();
                    const float baseAlpha = ((r.paint.color >> 24) & 0xFF) / 255.0f;
                    const float red = ((r.paint.color >> 16) & 0xFF) / 255.0f;
                    const float green = ((r.paint.color >> 8) & 0xFF) / 255.0f;
                    const float blue = (r.paint.color & 0xFF) / 255.0f;
                    pushRect(r.x, r.y, r.w, r.h, red, green, blue,
                             baseAlpha * r.opacity,
                             r.rotationRadians, r.pivotX, r.pivotY, r.clip);
                    break;
                }
                case CommandList::CommandKind::RoundedRect: {
                    const auto& r = command.GetRoundedRect();
                    const float baseAlpha = ((r.paint.color >> 24) & 0xFF) / 255.0f;
                    const float red = ((r.paint.color >> 16) & 0xFF) / 255.0f;
                    const float green = ((r.paint.color >> 8) & 0xFF) / 255.0f;
                    const float blue = (r.paint.color & 0xFF) / 255.0f;
                    pushRoundedRect(r.x, r.y, r.w, r.h,
                                    r.topLeft, r.topRight, r.bottomRight, r.bottomLeft,
                                    red, green, blue, baseAlpha * r.opacity,
                                    r.rotationRadians, r.pivotX, r.pivotY, r.clip);
                    break;
                }
                case CommandList::CommandKind::Arc:
                    pushArc(command.GetArc());
                    break;
                case CommandList::CommandKind::TexturedQuad:
                    break;
                }
            }
            flushSolidBatch();
            continue;
        }

        if (batch.kind == DrawBatchKind::LinearGradient ||
            batch.kind == DrawBatchKind::RadialGradient) {
            gradientVerts.reserve(batch.commandCount * 6);
            for (std::size_t index = batch.firstCommand;
                 index < batch.firstCommand + batch.commandCount;
                 ++index) {
                const auto& command = cmd.commands[index];
                float opacity = 1.0f;
                if (command.GetKind() == CommandList::CommandKind::Arc) {
                    const auto& arc = command.GetArc();
                    opacity = arc.opacity;
                    TessellateAnnularArc(
                        geometry, arc.centerX, arc.centerY, arc.radius,
                        arc.thickness, arc.startAngleRadians,
                        arc.sweepAngleRadians, arc.clip);
                } else if (command.GetKind() == CommandList::CommandKind::RoundedRect) {
                    const auto& rect = command.GetRoundedRect();
                    opacity = rect.opacity;
                    TessellateRoundedRect(
                        geometry, rect.x, rect.y, rect.w, rect.h,
                        rect.topLeft, rect.topRight, rect.bottomRight, rect.bottomLeft,
                        rect.clip, rect.rotationRadians, rect.pivotX, rect.pivotY);
                } else {
                    const auto& rect = command.GetRect();
                    opacity = rect.opacity;
                    TessellateRect(
                        geometry, rect.x, rect.y, rect.w, rect.h,
                        rect.clip, rect.rotationRadians, rect.pivotX, rect.pivotY);
                }
                for (const auto& vertex : geometry) {
                    gradientVerts.push_back({
                        static_cast<float>(vertex.x),
                        static_cast<float>(vertex.y),
                        static_cast<float>(vertex.x),
                        static_cast<float>(vertex.y),
                        opacity});
                }
            }
            flushGradientBatch(batch);
        }
    }
}

}
