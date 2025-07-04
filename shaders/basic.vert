#version 460 core

#extension GL_GOOGLE_include_directive : require

#include "particle_type.glsl"

layout(std140, binding = 1) buffer particlesSSBO {
    Particle particles[];
};

layout(location = 0) out vec4 out_Color;

vec3 positions[6] = vec3[6](
        vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0),
        vec3(1, 1, 0), vec3(-1, 1, 0), vec3(-1, -1, 0)
    );

void main() {
    float scale = 0.01f;
    vec3 finalPos = particles[gl_VertexIndex].pos;
    gl_Position = vec4(finalPos, 1.0f);
    out_Color = particles[gl_VertexIndex].color;
}
