#version 410 core

#include "../globals.glsl"

layout(points) in;
layout(line_strip, max_vertices = 16) out;

uniform vec3 pos = vec3(0.0);
uniform vec3 size = vec3(1.0);

void vertex(vec3 vPos) {
    gl_Position = projection*view*vec4(vPos, 1.0);
    EmitVertex();
}

// Emit points for a cube
void main() {
    vertex(pos);
    vertex(pos + size*vec3(1, 0, 0));
    vertex(pos + size*vec3(1, 0, 1));
    vertex(pos + size*vec3(0, 0, 1));
    vertex(pos);
    vertex(pos + size*vec3(0, 1, 0));
    vertex(pos + size*vec3(1, 1, 0));
    vertex(pos + size*vec3(1, 1, 1));
    vertex(pos + size*vec3(0, 1, 1));
    vertex(pos + size*vec3(0, 1, 0));
    EndPrimitive();

    vertex(pos + size*vec3(1, 0, 0));
    vertex(pos + size*vec3(1, 1, 0));
    EndPrimitive();

    vertex(pos + size*vec3(1, 0, 1));
    vertex(pos + size*vec3(1, 1, 1));
    EndPrimitive();

    vertex(pos + size*vec3(0, 0, 1));
    vertex(pos + size*vec3(0, 1, 1));
    EndPrimitive();
}
