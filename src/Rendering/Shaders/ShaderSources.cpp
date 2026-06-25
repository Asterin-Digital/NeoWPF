#include <NeoWPF/Rendering/Shaders/ShaderSources.h>

namespace NeoWPF::Rendering {

const char* GetRectVertexShaderSource()
{
    return R"(
struct VSIn { float2 pos : POSITION; float4 col : COLOR; };
cbuffer CB { float2 viewport; };
struct PSIn { float4 pos : SV_POSITION; float4 col : COLOR; };
PSIn VSMain(VSIn input) {
    PSIn o;
    float2 ndc = (input.pos / viewport) * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    o.pos = float4(ndc.xy, 0.0f, 1.0f);
    o.col = input.col;
    return o;
}
)";
}

const char* GetRectPixelShaderSource()
{
    return R"(
struct PSIn { float4 pos : SV_POSITION; float4 col : COLOR; };
float4 PSMain(PSIn input) : SV_Target { return input.col; }
)";
}

} // namespace NeoWPF::Rendering
