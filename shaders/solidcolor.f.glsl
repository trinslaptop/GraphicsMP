#version 410 core

uniform vec3 color = vec3(1.0);
out vec4 fColorOut;

void main() {
    fColorOut = vec4(color, 1.0);
}