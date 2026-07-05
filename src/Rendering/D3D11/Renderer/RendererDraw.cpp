#include <NeoWPF/Rendering/Renderer.h>
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <Windows.h>

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

    for (const auto& r : cmd.rects) {
        const float baseAlpha = ((r.color >> 24) & 0xFF) / 255.0f;
        const float red = ((r.color >> 16) & 0xFF) / 255.0f;
        const float green = ((r.color >> 8) & 0xFF) / 255.0f;
        const float blue = (r.color & 0xFF) / 255.0f;
        const float alpha = baseAlpha * r.opacity;
        pushRect(r.x, r.y, r.w, r.h, red, green, blue, alpha);
    }

    if (!rectVerts.empty() && g_vs && g_ps && g_inputLayout && g_vertexBuffer) {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (SUCCEEDED(g_context->Map(g_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            const float vp[2] = {static_cast<float>(width), static_cast<float>(height)};
            memcpy(mapped.pData, vp, sizeof(vp));
            g_context->Unmap(g_constantBuffer.Get(), 0);
        }

        D3D11_MAPPED_SUBRESOURCE vmap = {};
        if (SUCCEEDED(g_context->Map(g_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vmap))) {
            memcpy(vmap.pData, rectVerts.data(), rectVerts.size() * sizeof(Vertex));
            g_context->Unmap(g_vertexBuffer.Get(), 0);
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
        g_context->Draw(static_cast<UINT>(rectVerts.size()), 0);
        g_context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
    }

    struct TextVertex { float x, y; float u, v; float r, g, b, a; };
    std::vector<TextVertex> textVerts;
    textVerts.reserve(cmd.texturedQuads.size() * 6);

    auto pushTextQuad = [&](float x, float y, float w, float h, float u0, float v0, float u1, float v1, float opacity) {
        textVerts.push_back({x, y, u0, v0, 1.0f, 1.0f, 1.0f, opacity});
        textVerts.push_back({x + w, y, u1, v0, 1.0f, 1.0f, 1.0f, opacity});
        textVerts.push_back({x + w, y + h, u1, v1, 1.0f, 1.0f, 1.0f, opacity});
        textVerts.push_back({x + w, y + h, u1, v1, 1.0f, 1.0f, 1.0f, opacity});
        textVerts.push_back({x, y + h, u0, v1, 1.0f, 1.0f, 1.0f, opacity});
        textVerts.push_back({x, y, u0, v0, 1.0f, 1.0f, 1.0f, opacity});
    };

    for (const auto& q : cmd.texturedQuads) {
        pushTextQuad(q.x, q.y, q.w, q.h, q.u0, q.v0, q.u1, q.v1, q.opacity);
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

        D3D11_MAPPED_SUBRESOURCE vmap = {};
        if (SUCCEEDED(g_context->Map(g_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vmap))) {
            memcpy(vmap.pData, textVerts.data(), textVerts.size() * sizeof(TextVertex));
            g_context->Unmap(g_vertexBuffer.Get(), 0);
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

        UINT vertexStart = 0;
        for (size_t qi = 0; qi < cmd.texturedQuads.size();) {
            uint32_t texId = cmd.texturedQuads[qi].textureId;
            size_t batchCount = 1;
            while (qi + batchCount < cmd.texturedQuads.size() && cmd.texturedQuads[qi + batchCount].textureId == texId) {
                ++batchCount;
            }

            ComPtr<ID3D11ShaderResourceView> srv = GetTextureShaderResourceView(texId);
            if (srv) {
                g_context->PSSetShaderResources(0, 1, srv.GetAddressOf());
                g_context->Draw(static_cast<UINT>(batchCount * 6), vertexStart);
            }

            vertexStart += static_cast<UINT>(batchCount * 6);
            qi += batchCount;
        }

        g_context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
    }
}

}
