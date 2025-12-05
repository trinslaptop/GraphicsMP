#version 410 core

out vec4 fColorOut;

uniform sampler2D clouds;
in vec2 fTexCoord;
in vec3 fPos;

uniform float time;

uniform vec3 eyePos;

uniform float scale = 512.0;

void main() {
    vec4 texel = texture(clouds, fTexCoord + mod(vec2(time), vec2(1440.0, 1200.0))/vec2(1440.0, 1200.0));
    
    if(texel.a < 0.5) {
        discard;
    }

    // We always draw clouds second to only the skybox, and they're in the distance, so we can get away with alpha
    fColorOut = vec4(vec3(texel), 1.0 - distance(eyePos.xz, fPos.xz)/(scale/2));
}