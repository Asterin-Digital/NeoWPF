#version 450

layout(push_constant) uniform PushConstants {
    vec2 viewport;
} constants;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 0) out vec4 outColor;

void main()
{
    vec2 halfPixel = vec2(0.5 / constants.viewport.x, 0.5 / constants.viewport.y);
    vec2 ndc = ((inPosition + halfPixel) / constants.viewport) * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    gl_Position = vec4(ndc, 0.0, 1.0);
    outColor = inColor;
}
