#include <NeoWPF/Rendering/ShaderSources.h>

namespace NeoWPF::Rendering::D3D11 {

const char* GetRectPixelShaderSource()
{
    return R"(
struct PSIn { float4 pos : SV_POSITION; float4 col : COLOR; };
float4 PSMain(PSIn input) : SV_Target { return input.col; }
)";
}

const char* GetTextPixelShaderSource()
{
    return R"(
Texture2D tex : register(t0);
SamplerState samp : register(s0);
struct PSIn { float4 pos : SV_POSITION; float2 uv : TEXCOORD; float4 col : COLOR; };
float4 PSMain(PSIn input) : SV_Target {
    float4 sampled = tex.Sample(samp, input.uv);
    return sampled * input.col;
}
)";
}

} // namespace NeoWPF::Rendering::D3D11
