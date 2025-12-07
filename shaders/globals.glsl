#pragma once

layout(std140) uniform Globals {
    mat4 projection;
    mat4 view;
    float time;
    uniform vec3 eyePos;
};