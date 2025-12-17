#version 410 core

#include "globals.glsl"
#include "daylight.glsl"
#include "util.glsl"

// Since GLSL lacks enums, use constants and include the same file on the C++ side in glutils.hpp
const int
#include "RenderPass.glsl"
;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowTexture;

// Point light
uniform vec3 torchColor;
uniform vec3 torchPos1;
uniform vec3 torchPos2;

uniform bool lit = true;

uniform vec4 tint = vec4(1.0, 1.0, 1.0, 1.0);

in vec2 fTexCoord;
in vec3 fNormal;
in vec3 fPos;
in vec4 fPosLS;

out vec4 fColorOut;

uniform float frameTime = 1.0;
uniform uint frameCount = 1;

/// Compute shadow coverage
float shadow() {
    // Do perspective division and convert to normalized device coordinates
    vec3 proj = 0.5*(fPosLS.xyz/fPosLS.w) + 0.5;

    if(proj.z > 1.0) {
        return 0.0;
    }

    float bias = max(0.008*(1.0 - dot(fNormal, getSunDirection())), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0/textureSize(shadowTexture, 0);
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            shadow += proj.z - bias > texture(shadowTexture, proj.xy + texelSize*vec2(x, y)).r  ? 1.0 : 0.0;        
        }
    }

    return min(shadow/9.0, getSunIntensity());
}

/// Gets the diffuse texture for this fragment, applies tint and frame animation
vec4 diffuse() {
    vec2 fTexCoord1 = fTexCoord/vec2(1.0, frameCount) + vec2(0.0, floor(time/frameTime)/frameCount);
    vec2 fTexCoord2 = fTexCoord/vec2(1.0, frameCount) + vec2(0.0, mod(floor(time/frameTime) + 1.0, frameCount)/frameCount);

    // Lerp between animation frames
    return tint*mix(texture(diffuseTexture, fTexCoord1), texture(diffuseTexture, fTexCoord2), mod(time, frameTime)/frameTime);
}

/// Don't use this directly, instead call one of the specific light types below
vec4 phong(const vec3 L, const vec3 lColor) {
    vec3 N = fNormal;
    
    // Flip backface normals
    if(!gl_FrontFacing) N *= -1;
    
    vec3 V = normalize(eyePos - fPos);
    vec3 R = -L + 2*dot(N, L)*N;

    vec4 diffuseTexel = diffuse(); // tint*texture(diffuseTexture, fTexCoord);
    vec3 diffuse = vec3(diffuseTexel)*lColor*max(dot(L, N), 0);

    vec3 ambient = vec3(diffuseTexel)*0.35*lColor;

    vec4 specularTexel = texture(specularTexture, fTexCoord);
    vec3 specular = max(vec3(specularTexel)*lColor*pow(max(dot(V, R), 0.0), 32.0*specularTexel.a), vec3(0.0, 0.0, 0.0));

    return clamp(vec4((1.0 - shadow())*(diffuse + specular) + ambient, 1.0), vec4(0.0), vec4(1.0));
}

vec4 attenuate(const vec3 lPos, const vec4 i, const vec3 weights) {
    float d = max(1.0, length(lPos - fPos));
    return vec4(min(vec3(i), vec3(i)/(weights.x + weights.y*d + weights.z*d*d)), 1.0);
}

/// Calculate illumination from a spotlight
/// Based on https://learnopengl.com/Lighting/Light-casters
/// Radius is in radians
vec4 spot_light(const vec3 lPos, const vec3 lDirection, const float radius, const vec3 lColor, const vec3 lAttenuation) {
    vec3 L = normalize(lPos - fPos);

    if(dot(L, -normalize(lDirection)) > cos(radius)) {
        return attenuate(lPos, phong(L, lColor), lAttenuation);
    } else {
        return vec4(0.0, 0.0, 0.0, 1.0); // Outside of cone
    }
}

// Calculate illumination from a single point source
vec4 point_light(const vec3 lPos, const vec3 lColor, const vec3 lAttenuation) {
    return attenuate(lPos, phong(normalize(lPos - fPos), lColor), lAttenuation);
}

/// Calculate illumination from a constant directional light (e.g. distant sky) 
vec4 directional_light(const vec3 lDirection, const vec3 lColor) {
    return phong(-normalize(lDirection), lColor);
}

void main() {
    vec4 diffuseTexel = diffuse();

    // Ignore low alpha for cutout textures
    if(diffuseTexel.a < 0.5) {
        discard;
    }

    if(pass == PRIMARY_PASS) {
        if(lit) {
            // Compute Phong shading for all three lights
            // Point and spot lights are distance attenuated, but sun light isn't, so reduce it by a constant factor so it isn't overpowering
            // (That could be accounted for in the color itself, but this seems more consistent and can be varied with time)
            fColorOut = 
                getSunIntensity()*directional_light(getSunDirection(), getSunColor())
                + point_light(torchPos1, torchColor, vec3(0.0, 0.005, 0.01))
                + point_light(torchPos2, torchColor, vec3(0.0, 0.005, 0.01))
            ;
        } else {
            fColorOut = diffuseTexel;
        }
    }

    // gl_FragDepth = gl_FragCoord.z; (Implicit!)
}
