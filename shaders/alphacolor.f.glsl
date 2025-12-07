#version 410 core

uniform vec4 color = vec4(1.0);
out vec4 fColorOut;

void main() {
    fColorOut = color;
}