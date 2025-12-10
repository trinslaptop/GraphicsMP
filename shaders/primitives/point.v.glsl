#version 410 core

layout(std140) uniform Globals {
    mat4 projection;
    mat4 view;
    float time;
    vec3 eyePos;
};

uniform vec3 pos;
uniform float size;

void main() {
    gl_Position = projection*view*vec4(pos, 1.0);

    // Scale size with distance
    gl_PointSize = size/gl_Position.w;
}