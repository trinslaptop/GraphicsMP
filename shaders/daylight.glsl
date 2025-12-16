#pragma once

#include "globals.glsl"
#include "util.glsl"

const float DAY_LENGTH = 20.0;

float sunmod(float amplitude, float bias) {
    return clamp(amplitude*sin(2.0*PI*time/DAY_LENGTH) + bias, 0.0, 1.0);
}

float getSunIntensity() {
    return sunmod(2.0, 0.2);
}

float getSkyboxMixer() {
    return 1.0 - sunmod(-2.0, 0.2);
}

vec3 getCloudColor(vec2 coord) {
    return mix(
        mix(vec3(0.25, 0.25, 0.25), vec3(1.0, 1.0, 1.0), sunmod(2, 0.5)),
        mix(vec3(1.0, 0.83, 0.71), vec3(0.9647, 0.702, 1.0), noise(coord/50.0)),
        clamp(10.0*cos(4.0*PI*time/DAY_LENGTH) - 9.0, 0.0, 1.0)
    );
}

vec3 getSunDirection() {
    return -vec3(0.0, sin(2*PI*time/DAY_LENGTH), cos(2*PI*time/DAY_LENGTH));
}

// vec3 getSunColor() {

// }