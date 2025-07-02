#version 460 core

vec3 positions[6] = vec3[6](
        vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0),
        vec3(1, 1, 0), vec3(-1, 1, 0), vec3(-1, -1, 0)
    );

void main() {
    float scale = 1.0f;
    vec3 scaledPos = scale * positions[gl_VertexIndex];
    gl_Position = vec4(scaledPos, 1.0f);
}
