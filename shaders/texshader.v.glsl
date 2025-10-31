#version 410 core

uniform mat4 modelMatrix;
uniform mat4 vpMatrix;
uniform mat3 normalMatrix;

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexCoord;

out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fPos;

void main() {
    // Output position and forward rest of work to fragment shader
    gl_Position = vpMatrix*modelMatrix*vec4(vPos, 1.0);
    fTexCoord = vTexCoord;
    fNormal = normalMatrix*vNormal;
    fPos = vec3(modelMatrix*vec4(vPos, 1.0));
}