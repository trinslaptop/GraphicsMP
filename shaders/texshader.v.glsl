#version 410 core

#include "globals.glsl"

// Since GLSL lacks enums, use constants and include the same file on the C++ side in glutils.hpp
const int
#include "RenderPass.glsl"
;

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexCoord;

uniform float oscillation;

out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fPos;
out vec4 fPosLS;

void main() {
    vec3 vOffsetPos = vPos + vec3(0.0625*oscillation*cos(time + length(vec3(modelMatrix*vec4(vPos, 1.0)))));
    fPos = vec3(modelMatrix*vec4(vOffsetPos, 1.0));
    fTexCoord = vTexCoord;
    fNormal = normalMatrix*vNormal;

    // Output position and forward rest of work to fragment shader
    if(pass == PRIMARY_PASS) {
        gl_Position = projection*view*modelMatrix*vec4(vOffsetPos, 1.0);
    } else {
        gl_Position = lsm*modelMatrix*vec4(vOffsetPos, 1.0);
    }
    fPosLS = lsm*vec4(fPos, 1.0);
}