#version 410 core

#include "../globals.glsl"

uniform float height = 62.0;
uniform float scale = 512.0;

in vec3 vPos;

out vec2 fTexCoord;
out vec3 fPos;

void main() {
    // Center a large quad around camera but offset the texture so it doesn't move with you
    vec3 pos = vPos*scale + vec3(-scale/2, height, -scale/2) + eyePos*vec3(1.0, 0.0, 1.0);
    fTexCoord = vPos.xz + eyePos.xz/scale;
    fPos = pos;

    gl_Position = projection*view*vec4(pos, 1.0f);
}