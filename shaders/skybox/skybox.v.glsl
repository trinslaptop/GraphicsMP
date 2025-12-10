#version 410 core

layout(std140) uniform Globals {
    mat4 projection;
    mat4 view;
    float time;
    vec3 eyePos;
};

in vec3 vPos;

out vec3 fTexCoord;

void main() {
    fTexCoord = vPos;
    
    // Remove translation from view matrix
    gl_Position = (projection*mat4(mat3(view))*vec4(vPos, 1.0)).xyww;
}