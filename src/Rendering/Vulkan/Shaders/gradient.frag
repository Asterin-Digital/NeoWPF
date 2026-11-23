#version 450

layout(push_constant) uniform PushConstants {
    vec2 viewport;
    vec4 mapping0;
    vec4 mapping1;
    vec4 gradient;
} constants;

layout(set = 0, binding = 0) uniform sampler2D rampAtlas;

layout(location = 0) in vec2 inWorldPosition;
layout(location = 1) in float inOpacity;
layout(location = 0) out vec4 outColor;

float ApplySpread(float coordinate)
{
    if (constants.gradient.y < 0.5) {
        return clamp(coordinate, 0.0, 1.0);
    }
    if (constants.gradient.y < 1.5) {
        float repeated = mod(abs(coordinate), 2.0);
        return repeated <= 1.0 ? repeated : 2.0 - repeated;
    }
    return fract(coordinate);
}

void main()
{
    float coordinate = 0.0;
    if (constants.gradient.x < 0.5) {
        coordinate = constants.mapping0.x * inWorldPosition.x +
                     constants.mapping0.z * inWorldPosition.y +
                     constants.mapping1.x;
    } else {
        vec2 ellipsePosition = vec2(
            constants.mapping0.x * inWorldPosition.x +
            constants.mapping0.z * inWorldPosition.y +
            constants.mapping1.x,
            constants.mapping0.y * inWorldPosition.x +
            constants.mapping0.w * inWorldPosition.y +
            constants.mapping1.y);
        vec2 focalPosition = vec2(constants.mapping1.z, constants.mapping1.w);
        coordinate = length(ellipsePosition - focalPosition);
    }

    coordinate = ApplySpread(coordinate);
    vec2 rampUv = vec2(coordinate, constants.gradient.z);
    vec4 sampled = texture(rampAtlas, rampUv);
    outColor = vec4(sampled.rgb, sampled.a * constants.gradient.w * inOpacity);
}
