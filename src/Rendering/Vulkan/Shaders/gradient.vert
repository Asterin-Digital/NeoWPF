#version 450

layout(push_constant) uniform PushConstants {
    vec2 viewport;
    vec4 mapping0;
    vec4 mapping1;
    vec4 gradient;
} constants;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inWorldPosition;
layout(location = 2) in float inOpacity;
layout(location = 0) out vec2 outWorldPosition;
layout(location = 1) out float outOpacity;

void main()
{
    vec2 halfPixel = vec2(0.5 / constants.viewport.x, 0.5 / constants.viewport.y);
    vec2 ndc = ((inPosition + halfPixel) / constants.viewport) * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    gl_Position = vec4(ndc, 0.0, 1.0);
    outWorldPosition = inWorldPosition;
    outOpacity = inOpacity;
}
