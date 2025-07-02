#version 460 core

#extension GL_GOOGLE_include_directive : require

#include "uniforms.glsl"

layout(location = 0) out vec4 out_Color;

layout(std140, binding = 0) uniform UniformBuffer {
    UniformBufferStruct uniforms;
};

#define PI 3.14159265359f

void main() {
    float r = 0.5 * sin(uniforms.runningTimeSec) + 0.5;
    float g = 0.5 * cos(uniforms.runningTimeSec) + 0.5;
    float b = 0.5 * sin(uniforms.runningTimeSec + PI) + 0.5;
    out_Color = vec4(r, g, b, 0.5f);
}
