#version 410 core

layout(quads, equal_spacing, ccw) in;

layout(std140) uniform Globals {
    mat4 projection;
    mat4 view;
    float time;
    vec3 eyePos;
};

uniform mat4 modelMatrix;

out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fPos;

vec4 bc(vec4 p0, vec4 p1, const vec4 p2, const vec4 p3, const float t) {
    return pow(1 - t, 3)*p0 + 3*pow(1 - t, 2)*t*p1 + 3*(1 - t)*pow(t, 2)*p2 + pow(t, 3)*p3;
}

vec4 dbc(vec4 p0, vec4 p1, const vec4 p2, const vec4 p3, const float t) {
    return -3*p0*pow(t - 1, 2) + 6*p1*t*(t - 1) + 3*p1*pow(t - 1, 2) - 3*p2*pow(t, 2) - 6*p2*t*(t - 1) + 3*p3*pow(t, 2);
}

void main() {
    // Patch coordinates
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // Patch vertices
    vec4 p00 = gl_in[ 0].gl_Position, p01 = gl_in[ 1].gl_Position, p02 = gl_in[ 2].gl_Position, p03 = gl_in[ 3].gl_Position,
         p10 = gl_in[ 4].gl_Position, p11 = gl_in[ 5].gl_Position, p12 = gl_in[ 6].gl_Position, p13 = gl_in[ 7].gl_Position,
         p20 = gl_in[ 8].gl_Position, p21 = gl_in[ 9].gl_Position, p22 = gl_in[10].gl_Position, p23 = gl_in[11].gl_Position,
         p30 = gl_in[12].gl_Position, p31 = gl_in[13].gl_Position, p32 = gl_in[14].gl_Position, p33 = gl_in[15].gl_Position
    ;

    // Compute position
    vec4 p = bc(
        bc(p00, p01, p02, p03, u),
        bc(p10, p11, p12, p13, u),
        bc(p20, p21, p22, p23, u),
        bc(p30, p31, p32, p33, u),
        v
    );

    // Compute partial derivatives
    vec3 partu = vec3(bc(
        dbc(p00, p01, p02, p03, u),
        dbc(p10, p11, p12, p13, u),
        dbc(p20, p21, p22, p23, u),
        dbc(p30, p31, p32, p33, u),
        v
    ));

    vec3 partv = vec3(bc(
        dbc(p00, p10, p20, p30, v),
        dbc(p01, p11, p21, p31, v),
        dbc(p02, p12, p22, p32, v),
        dbc(p03, p13, p23, p33, v),
        u
    ));

    // Output to fragment shader
    fNormal = normalize(cross(partu, partv));
    fPos = vec3(modelMatrix*p);
    fTexCoord = p.xz;

    gl_Position = projection*view*modelMatrix*p;
}