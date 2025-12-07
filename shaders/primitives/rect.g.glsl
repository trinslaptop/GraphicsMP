#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform vec2 pos;
uniform vec2 size;

void uivertex(vec2 offset) {
    gl_Position = vec4(2.0*pos - vec2(1.0, 1.0) + 2.0*offset, 0.0, 1.0);
    EmitVertex();
}

void main() {
    uivertex(vec2(0.0, 0.0));
    uivertex(vec2(0.0, size.y));
    uivertex(vec2(size.x, 0.0));
    uivertex(vec2(size.x, size.y));
    EndPrimitive();
}