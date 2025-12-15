#pragma once

#include "globals.glsl"
#include "util.glsl"

const float DAY_LENGTH = 15.0;

float getSunIntensity() {
    return 0.95*pow(sin(PI*time/DAY_LENGTH), 2.0) + 0.05;
}

float getSkyboxMixer() {
    return clamp(3.0*pow(sin(PI*time/DAY_LENGTH), 2.0) - 1.0, 0.0, 1.0);
}

vec3 getCloudColor() {
    return mix(
        mix(vec3(0.25, 0.25, 0.25), vec3(1.0, 1.0, 1.0), clamp(2.0*pow(sin(PI*time/DAY_LENGTH), 2.0), 0.0, 1.0)),
        mix(vec3(1.0, 0.83, 0.71), vec3(0.9647, 0.702, 1.0), noise(gl_FragCoord.xy)),
        clamp(1.5*sin(4.0*PI*(time - DAY_LENGTH/8.0)/DAY_LENGTH) - 0.5, 0.0, 1.0)
    );
}



// vec3 getSunColor() {

// }