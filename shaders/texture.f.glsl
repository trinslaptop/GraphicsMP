#version 410 core

uniform sampler2D tex;
uniform bool grayscale = false;

in vec2 fTexCoord;
out vec4 fColorOut;

void main() {
    if(grayscale) {
        fColorOut = vec4(vec3(texture(tex, fTexCoord).r), 1.0);
    } else {
        fColorOut = texture(tex, fTexCoord);
    }
}