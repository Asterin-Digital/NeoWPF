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

const char* GetGradientPixelShaderSource()
{
    return R"(
Texture2D rampAtlas : register(t0);
SamplerState rampSampler : register(s0);
cbuffer GradientConstants {
    float2 viewport;
    float4 mapping0;
    float4 mapping1;
    float4 gradient;
};
struct PSIn {
    float4 pos : SV_POSITION;
    float2 worldPosition : TEXCOORD;
};
float ApplySpread(float coordinate) {
    if (gradient.y < 0.5f) {
        return saturate(coordinate);
    }
    if (gradient.y < 1.5f) {
        float repeated = fmod(abs(coordinate), 2.0f);
        return repeated <= 1.0f ? repeated : 2.0f - repeated;
    }
    return frac(coordinate);
}
float4 PSMain(PSIn input) : SV_Target {
    float coordinate = 0.0f;
    if (gradient.x < 0.5f) {
        coordinate = mapping0.x * input.worldPosition.x +
                     mapping0.z * input.worldPosition.y +
                     mapping1.x;
    } else {
        float2 ellipsePosition = float2(
            mapping0.x * input.worldPosition.x +
            mapping0.y * input.worldPosition.y +
            mapping1.x,
            mapping0.z * input.worldPosition.x +
            mapping0.w * input.worldPosition.y +
            mapping1.y);
        float2 focalPosition = float2(mapping1.z, mapping1.w);
        coordinate = length(ellipsePosition - focalPosition);
    }
    float4 sampled = rampAtlas.Sample(rampSampler, float2(ApplySpread(coordinate), gradient.z));
    return float4(sampled.rgb, sampled.a * gradient.w);
}
)";
}

}
