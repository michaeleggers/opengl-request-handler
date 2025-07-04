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

    float r = 0.5f * sin(runningTimeSec) + 0.5f;
    float g = 0.5f * cos(runningTimeSec) + 0.5f;
    float b = 0.5f * cos(runningTimeSec + PI / 2.0f) + 0.5f;

    vec4 finalColor = in_Color;
    //finalColor.r *= r;
    //finalColor.g *= g;
    finalColor.b *= b;

    out_Color = finalColor;
}
