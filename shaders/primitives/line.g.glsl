#version 410 core

layout(points) in;
layout(line_strip, max_vertices = 2) out;

uniform vec3 pos1 = vec3(0.0);
uniform vec3 pos2 = vec3(1.0);

uniform mat4 vpMatrix;

// Emit a single line segment
void main() {
    gl_Position = vpMatrix*vec4(pos1, 1.0);
    EmitVertex();

    gl_Position = vpMatrix*vec4(pos2, 1.0);
    EmitVertex();

    EndPrimitive();
}
