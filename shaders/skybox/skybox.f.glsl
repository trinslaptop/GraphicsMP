#version 410 core

uniform samplerCube skybox;

in vec3 fTexCoord;

out vec4 fColorOut;

void main() {
    // fColorOut = vec4(fTexCoord, 1.0);
    fColorOut = texture(skybox, fTexCoord);
}