#pragma once

const float PI = 3.141592654;

float rand(vec2 v) { 
	return fract(sin(dot(v, vec2(12.9898, 78.233))) * 43758.5453123);
}

float noise(vec2 v){
	vec2 i = floor(v);
	vec2 f = fract(v);
	vec2 u = f*f*(3.0 - 2.0*f);
	float r = mix(
		mix(rand(i),                 rand(i + vec2(1.0,0.0)), u.x),
		mix(rand(i + vec2(0.0,1.0)), rand(i + vec2(1.0,1.0)), u.x),
        u.y
    );

	return r*r;
}