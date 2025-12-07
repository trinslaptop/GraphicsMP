#version 410 core

#include "../globals.glsl"

uniform vec3 pos;
uniform float size;

void main() {
    gl_Position = projection*view*vec4(pos, 1.0);

    // Scale size with distance
    gl_PointSize = size/gl_Position.w;
}