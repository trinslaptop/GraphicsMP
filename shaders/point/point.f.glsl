#version 410 core

uniform vec3 color;
out vec4 fColorOut;

void main() {
    fColorOut = vec4(color, 1.0);
}