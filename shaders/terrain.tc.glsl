#version 410 core

layout(vertices = 4) out;

uniform float size;

void main() {
    // Pass attributes through
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // Invocation zero controls patch tessellation levels
    if(gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = size;
        gl_TessLevelOuter[1] = size;
        gl_TessLevelOuter[2] = size;
        gl_TessLevelOuter[3] = size;

        gl_TessLevelInner[0] = size;
        gl_TessLevelInner[1] = size;
    }
}