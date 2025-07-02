#version 460 core

#extension GL_GOOGLE_include_directive : require

#include "uniforms.glsl"

layout(location = 0) out vec4 out_Color;

layout(std140, binding = 0) uniform UniformBuffer {
    UniformBufferStruct uniforms;
};

#define PI 3.14159265359f

void main() {
    float runningTimeSec = uniforms.runningTimeSec;
    uint width = uniforms.windowWidth;
    uint height = uniforms.windowHeight;

    float r = sin(runningTimeSec);
    float g = cos(runningTimeSec);
    float b = sin(runningTimeSec + PI);

    uint uOffsetR = uint(float(width) * r);
    uint uR = uint(gl_FragCoord.x);
    uint uShiftedR = (uR + uOffsetR) % width;
    float fR = float(uShiftedR) / float(width);

    uint uOffsetG = uint(float(height) * g);
    uint uG = uint(gl_FragCoord.y);
    uint uShiftedG = (uG + uOffsetG) % height;
    float fG = float(uShiftedG) / float(height);

    uint uOffsetB = uint(float(width * height) * b);
    uint uB = uint(float(width) * gl_FragCoord.y + gl_FragCoord.x);
    uint uShiftedB = (uB + uOffsetB) % (width * height);
    float fB = float(uShiftedB) / float(width * height);

    float normR = clamp((gl_FragCoord.x + r) / float(width), 0.0f, 1.0f);
    float normG = clamp(gl_FragCoord.y / float(height), 0.0f, 1.0f);
    float normB = 0.0;

    out_Color = vec4(r, g, b, 0.5f);
    out_Color = vec4(fR, fG, fB, 0.5f);
}
