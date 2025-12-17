#pragma once

layout(std140) uniform Globals {
    mat4 projection;
    mat4 view;
    mat4 lsm;
    float time;
    vec3 eyePos;
    int pass;
    float DAY_LENGTH;
};