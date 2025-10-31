#version 410 core

uniform mat4 vpMatrix;

in vec3 vPos;

out vec3 fTexCoord;

void main() {
    fTexCoord = vPos;
    gl_Position = vpMatrix*vec4(vPos, 1.0);
}