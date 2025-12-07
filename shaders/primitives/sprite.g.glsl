#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 projMatrix;

uniform int sprite;
uniform vec3 pos;

out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fPos;

void main {
    // TODO: finish
}