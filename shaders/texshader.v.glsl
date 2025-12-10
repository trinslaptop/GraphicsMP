#version 410 core

layout(std140) uniform Globals {
    mat4 projection;
    mat4 view;
    float time;
    vec3 eyePos;
};

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexCoord;

uniform float oscillation;

out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fPos;

void main() {
    vec3 vOffsetPos = vPos + vec3(0.0625*oscillation*cos(time + length(vec3(modelMatrix*vec4(vPos, 1.0)))));
    // Output position and forward rest of work to fragment shader
    gl_Position = projection*view*modelMatrix*vec4(vOffsetPos, 1.0);
    fTexCoord = vTexCoord;
    fNormal = normalMatrix*vNormal;
    fPos = vec3(modelMatrix*vec4(vOffsetPos, 1.0));
}