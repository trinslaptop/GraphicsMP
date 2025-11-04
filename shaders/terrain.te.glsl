#version 410 core

layout(quads, equal_spacing, ccw) in;

uniform mat4 modelMatrix;
uniform mat4 vpMatrix;

out float height;

void main() {
    // Patch coordinates
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // Patch vertices
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    // Blerp position
    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p = (p1 - p0) * v + p0;

    p.y = 3*pow(sin(0.0005*(p.x*p.x + p.z*p.z)), 2) + pow(sin(0.1*p.x), 2) + 0.0625;

    gl_Position = vpMatrix*modelMatrix*p;

    height = p.y*0.8;
}