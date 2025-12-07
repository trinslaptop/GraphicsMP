#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

#include "../globals.glsl"

// Since GLSL lacks enums, use constants and include the same file on the C++ side in PrimitiveRenderer.hpp
const int
#include "SpriteMode.glsl"
;

uniform sampler2D diffuseTexture;

uniform int sprite;
uniform vec3 pos;
uniform int mode;
uniform float size;

out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fPos;

void uivertex(vec2 offset, vec2 texOffset) {
    gl_Position = vec4(vec2(pos) - vec2(1.0, 1.0) + offset, 0.0, 1.0);
    fTexCoord = vec2(sprite % 16, 15 - sprite/16)/16.0 + texOffset;
    EmitVertex();
}

void main() {
    float texSpan = 1.0/16.0;
    vec2 texCoord = vec2(0);

    switch(mode) {
        case PARTICLE:
            break;

        case UI_ANCHOR_CORNER:
            fPos = vec3(0.0, 0.0, 0.0);
            fNormal = vec3(0.0, 0.0, 1.0);

            uivertex(vec2(0.0, 0.0), vec2(0.0, 0.0));
            uivertex(vec2(0.0, size), vec2(0.0, texSpan));
            uivertex(vec2(size, 0.0), vec2(texSpan, 0.0));
            uivertex(vec2(size, size), vec2(texSpan, texSpan));
            EndPrimitive();

            break;

        case UI_ANCHOR_CENTER:
        default:
            fPos = vec3(0.0, 0.0, 0.0);
            fNormal = vec3(0.0, 0.0, 1.0);

            uivertex(vec2(-size/2.0, -size/2.0), vec2(0.0, 0.0));
            uivertex(vec2(-size/2.0, size/2.0), vec2(0.0, texSpan));
            uivertex(vec2(size/2.0, -size/2.0), vec2(texSpan, 0.0));
            uivertex(vec2(size/2.0, size/2.0), vec2(texSpan, texSpan));
            EndPrimitive();

            break;
    }
}