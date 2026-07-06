#include <NeoWPF/Rendering/ShaderSources.h>

namespace NeoWPF::Rendering::D3D11 {

const char* GetRectVertexShaderSource()
{
    return R"(
struct VSIn { float2 pos : POSITION; float4 col : COLOR; };
cbuffer CB { float2 viewport; };
struct PSIn { float4 pos : SV_POSITION; float4 col : COLOR; };
PSIn VSMain(VSIn input) {
    PSIn o;
    float2 halfPixel = float2(0.5f / viewport.x, 0.5f / viewport.y);
    float2 ndc = ((input.pos + halfPixel) / viewport) * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    o.pos = float4(ndc.xy, 0.0f, 1.0f);
    o.col = input.col;
    return o;
}
)";
}

const char* GetTextVertexShaderSource()
{
    return R"(
struct VSIn { float2 pos : POSITION; float2 uv : TEXCOORD; float4 col : COLOR; };
cbuffer CB { float2 viewport; };
struct PSIn { float4 pos : SV_POSITION; float2 uv : TEXCOORD; float4 col : COLOR; };
PSIn VSMain(VSIn input) {
    PSIn o;
    float2 ndc = (input.pos / viewport) * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    o.pos = float4(ndc.xy, 0.0f, 1.0f);
    o.uv = input.uv;
    o.col = input.col;
    return o;
}
)";
}

}
