#version 410 core

out vec4 fColorOut;

in float height;

void main() {
    fColorOut = vec4(0.0, 0.5, height, 1.0);
}