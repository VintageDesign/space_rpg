#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

// 2x3 affine view transform, world -> clip:
// clip = axisX * pos.x + axisY * pos.y + origin.
layout(push_constant) uniform PushConstants {
    vec2 axisX;
    vec2 axisY;
    vec2 origin;
} pc;

void main() {
    vec2 clip = pc.axisX * inPosition.x + pc.axisY * inPosition.y + pc.origin;
    gl_Position = vec4(clip, 0.0, 1.0);
    fragColor = inColor;
}
