#version 460 core

#extension GL_GOOGLE_include_directive : require

#include "uniforms.glsl"

layout(location = 0) in vec4 in_Color;

layout(location = 0) out vec4 out_Color;

layout(std140, binding = 0) uniform UniformBuffer {
    UniformBufferStruct uniforms;
};

#define PI 3.14159265359f

void main() {
    float runningTimeSec = uniforms.runningTimeSec;
    uint width = uniforms.windowWidth;
    uint height = uniforms.windowHeight;

    out_Color = in_Color;
}
