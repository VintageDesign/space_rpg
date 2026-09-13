#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

// Maps world pixels to NDC: ndc = pos * scale + offset.
layout(push_constant) uniform PushConstants {
    vec2 scale;
    vec2 offset;
} pc;

void main() {
    gl_Position = vec4(inPosition * pc.scale + pc.offset, 0.0, 1.0);
    fragColor = inColor;
}
