#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

layout(std140) uniform Globals {
    mat4 projection;
    mat4 view;
    float time;
    vec3 eyePos;
};

// Since GLSL lacks enums, use constants and include the same file on the C++ side in PrimitiveRenderer.hpp
const int
UI_ANCHOR_CENTER = 0,

/// Renders a sprite flat on the UI with it's lower left corner at the given position (ignores z pos)
UI_ANCHOR_CORNER = 1,

/// Renders a billboarded sprite in the 3D world
PARTICLE = 2,

/// Computes size without actually drawing sprite
HIDDEN = 3
;

uniform sampler2D diffuseTexture;

uniform int sprite;
uniform vec3 pos;
uniform int mode;
uniform float size;

out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fPos;

void vertex(vec2 offset, vec2 texOffset) {
    gl_Position = projection*(view*vec4(pos, 1.0) + vec4(offset/2.0, 0.0, 0.0));
    fTexCoord = vec2(sprite % 16, 15.0 - floor(sprite/16.0))/16.0 + texOffset;
    EmitVertex();
}

void uivertex(vec2 offset, vec2 texOffset) {
    gl_Position = vec4(2.0*vec2(pos) - vec2(1.0, 1.0) + offset, 0.0, 1.0);
    fTexCoord = vec2(sprite % 16, 15.0 - floor(sprite/16.0))/16.0 + texOffset;
    EmitVertex();
}

void main() {
    float texSpan = 1.0/16.0;

    switch(mode) {
        case PARTICLE:
            fPos = pos;
            fNormal = vec3(0.0, 0.0, 1.0);

            vertex(vec2(-size, -size), vec2(0.0, 0.0));
            vertex(vec2(-size, size), vec2(0.0, texSpan));
            vertex(vec2(size, -size), vec2(texSpan, 0.0));
            vertex(vec2(size, size), vec2(texSpan, texSpan));
            EndPrimitive();

            break;

        case UI_ANCHOR_CORNER:
            fPos = vec3(0.0, 0.0, 0.0);
            fNormal = vec3(0.0, 0.0, 1.0);

            uivertex(vec2(0.0, 0.0), vec2(0.0, 0.0));
            uivertex(vec2(0.0, 2.0*size), vec2(0.0, texSpan));
            uivertex(vec2(2.0*size, 0.0), vec2(texSpan, 0.0));
            uivertex(vec2(2.0*size, 2.0*size), vec2(texSpan, texSpan));
            EndPrimitive();

            break;

        case UI_ANCHOR_CENTER:
        default:
            fPos = vec3(0.0, 0.0, 0.0);
            fNormal = vec3(0.0, 0.0, 1.0);

            uivertex(vec2(-size, -size), vec2(0.0, 0.0));
            uivertex(vec2(-size, size), vec2(0.0, texSpan));
            uivertex(vec2(size, -size), vec2(texSpan, 0.0));
            uivertex(vec2(size, size), vec2(texSpan, texSpan));
            EndPrimitive();

            break;
    }
}