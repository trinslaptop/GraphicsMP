#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 8) out;

#include "../globals.glsl"
#include "../daylight.glsl"

out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fPos;
out vec4 fPosLS;

vec2 atlasSize = vec2(2, 2);

void vertex(vec3 pos, vec2 texOffset) {
    gl_Position = projection*(mat4(mat3(view))*vec4(pos, 1.0));
    fTexCoord = texOffset;
    EmitVertex();
}

void skyquad(vec3 direction, vec2 atlasOffset, float size) {
    // Sun moves in yz, so fix around x
    vec3 x = normalize(cross(direction, vec3(1.0, 0.0, 0.0)));
    vec3 y = normalize(cross(x, direction));

    vertex(direction - size*x - size*y, (atlasOffset + vec2(1.0, 0.0))/atlasSize);
    vertex(direction - size*x + size*y, (atlasOffset + vec2(0.0, 0.0))/atlasSize);
    vertex(direction + size*x - size*y, (atlasOffset + vec2(1.0, 1.0))/atlasSize);
    vertex(direction + size*x + size*y, (atlasOffset + vec2(0.0, 1.0))/atlasSize);
    EndPrimitive();
}

void main() {
    fNormal = vec3(0.0, 0.0, 1.0);
    fPos = vec3(0.0, 0.0, 0.0);
    fPosLS = vec4(0.0, 0.0, 0.0, 0.0);

    // Sun
    skyquad(-getSunDirection(), vec2(0.0, 0.0), 0.1);

    // Moon
    switch(int(time/DAY_LENGTH - 0.25) % 4) {
        case 0:
            // Full
            skyquad(getSunDirection(), vec2(1.0, 0.0), 0.05);
            break;
        case 1:
            // Waning
            skyquad(getSunDirection(), vec2(1.0, 1.0), 0.05);
            break;
        case 2:
            // New
            break;
        case 3:
            // Waxing
            skyquad(getSunDirection(), vec2(0.0, 1.0), 0.05);
            break;
    }
}