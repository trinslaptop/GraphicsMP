#version 410 core

uniform sampler2D tex;

in vec2 fTexCoord;
out vec4 fColorOut;

void main() {
    fColorOut = texture(tex, fTexCoord);
}