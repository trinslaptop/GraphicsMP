#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform vec2 pos;
uniform vec2 size;
uniform vec2 texCoord;
uniform vec2 texSpan;

out vec2 fTexCoord;

void uivertex(vec2 offset, vec2 texOffset) {
    gl_Position = vec4(2.0*pos - vec2(1.0, 1.0) + 2.0*offset, 0.0, 1.0);
    fTexCoord = texCoord + texOffset;
    EmitVertex();
}

void main() {
    uivertex(vec2(0.0, 0.0), vec2(0.0, 0.0));
    uivertex(vec2(0.0, size.y), vec2(0.0, texSpan.y));
    uivertex(vec2(size.x, 0.0), vec2(texSpan.x, 0.0));
    uivertex(vec2(size.x, size.y), texSpan);
    EndPrimitive();
}