#version 410 core

uniform mat4 vpMatrix;
uniform vec3 pos;
uniform float size;

void main() {
    gl_Position = vpMatrix*vec4(pos, 1.0);

    // Scale size with distance
    gl_PointSize = size/gl_Position.w;
}