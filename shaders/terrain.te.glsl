#version 410 core

layout(quads, equal_spacing, ccw) in;

uniform mat4 modelMatrix;
uniform mat4 vpMatrix;

out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fPos;

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

    float x = p.x, z = p.z;

    // Compute terrain y
    p.y = 3*pow(sin(0.0005*(x*x + z*z)), 2) + pow(sin(0.1*x), 2) + 0.0625;

    // Output to fragment shader
    fNormal = normalize(vec3(
        0.006*x*sin(0.0005*(x*x + z*z))*cos(0.0005*(x*x + z*z)) + 0.2*sin(0.1*x)*cos(0.1*x), // partial x
        -1.0,
        0.006*z*sin(0.0005*(x*x + z*z))*cos(0.0005*(x*x + z*z)) // partial z
    ));
    fPos = vec3(modelMatrix*p);
    fTexCoord = p.xz;

    gl_Position = vpMatrix*modelMatrix*p;
}