#version 410 core

#include "../globals.glsl"
#include "../daylight.glsl"

uniform samplerCube skybox0;
uniform samplerCube skybox1;

in vec3 fTexCoord;

out vec4 fColorOut;

/// Converts a vec3 direction into a vec2 latitude longitude
vec2 latlong(vec3 v) {
    return vec2(acos(v.y), atan2(v.x, v.z));
}

void main() {
    // fColorOut = vec4(fTexCoord, 1.0);
    float starmod = 2.0*abs(mod(0.25*time + rand(latlong(fTexCoord)), 1.0) - 0.5);
    fColorOut = vec4(mix(starmod*vec3(texture(skybox0, fTexCoord)), vec3(texture(skybox1, fTexCoord)), getSkyboxMixer()), 1.0);
}