#version 460 core

#extension GL_GOOGLE_include_directive : require

#include "uniforms.glsl"

layout(location = 0) out vec4 out_Color;

layout(binding = 0) uniform UniformBuffer {
    UniformBufferStruct uniforms;
};

void main() {
    out_Color = vec4(1.0f, 0.0f, 0.0f, 0.5f);
}
