#version 410 core

#include "../globals.glsl"

in vec3 vPos;

out vec3 fTexCoord;

void main() {
    fTexCoord = vPos;
    
    // Remove translation from view matrix
    gl_Position = (projection*mat4(mat3(view))*vec4(vPos, 1.0)).xyww;
}