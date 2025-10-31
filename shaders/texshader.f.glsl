#version 410 core

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform vec3 eyePos;
uniform vec3 lightColor;
uniform vec3 lightPos;

uniform bool lit;

in vec2 fTexCoord;
in vec3 fNormal;
in vec3 fPos;

out vec4 fColorOut;

void main() {
    vec4 diffuseTexel = texture(diffuseTexture, fTexCoord);

    // Ignore low alpha for cutout textures
    if(diffuseTexel.a < 0.5) {
        discard;
    }

    if(lit) {
        // Compute phong shading for point light
        vec3 N = fNormal;
        vec3 L = normalize(lightPos - fPos);
        vec3 V = normalize(eyePos - fPos);
        vec3 R = -L + 2*dot(N, L)*N;

        vec3 diffuse = vec3(diffuseTexel)*lightColor*max(dot(L, N), 0);

        vec3 ambient = vec3(diffuseTexel)*0.35*lightColor;

        vec4 specularTexel = texture(specularTexture, fTexCoord);
        vec3 specular = max(vec3(specularTexel)*lightColor*pow(max(dot(V, R), 0.0), 32.0*specularTexel.a), vec3(0.0, 0.0, 0.0));

        fColorOut = vec4(diffuse + specular + ambient, 1.0);
    } else {
        fColorOut = diffuseTexel;
    }
}
