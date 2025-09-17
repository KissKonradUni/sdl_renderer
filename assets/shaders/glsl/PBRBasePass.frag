#version 460 core

in vec2 fragmentUV;

out vec4 outputColor;

uniform sampler2D gDiffuse;
uniform sampler2D gNormal;
uniform sampler2D gPosition;
uniform sampler2D gAORoughnessMetallic;

uniform vec4 screenSize;

in vec3 cameraPosition;
in vec3 cameraDirection;
in mat4 cameraMatrix;
in mat4 cameraInverseMatrix;

// Direcitional light
vec2 calculateSkyboxLighting(vec3 position, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(vec3(0.0, 1.0, 0.0));
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 256.0);

    return vec2(diff, spec);
}

void main()
{
    vec3 albedo = texture(gDiffuse, fragmentUV).rgb;
    vec3 normal = normalize(texture(gNormal, fragmentUV).rgb * 2.0 - 1.0);
    vec3 position = texture(gPosition, fragmentUV).rgb;
    vec4 aoRoughnessMetallic = texture(gAORoughnessMetallic, fragmentUV);

    float ao        = aoRoughnessMetallic.r;
    float roughness = aoRoughnessMetallic.g;
    float metallic  = aoRoughnessMetallic.b;

    vec4 skyboxColor = vec4(0.5, 0.7, 1.0, 1.0);
    if (length(position) <= 0.001) {
        outputColor = skyboxColor;
        return;
    }

    vec3 lightPos   = vec3(-2.5, 3.0, -0.5);

    vec3 lightDir   = normalize(lightPos - position);
    vec3 viewDir    = normalize(cameraPosition - position);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = albedo * diff;

    float specPower = mix(16.0, 256.0, 1.0 - roughness);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), specPower);
    vec3 specular = vec3(0.5) * spec;

    float strength = 3.0;
    float attenuation = 1.0 / length(lightPos - position) * strength;
    attenuation = clamp(attenuation, 0.0, 1.0);

    diffuse *= attenuation;
    specular *= attenuation;

    vec2 skyboxLighting = calculateSkyboxLighting(position, normal, viewDir);
    diffuse += albedo * skyboxColor.xyz * skyboxLighting.x;
    specular += skyboxColor.xyz * skyboxLighting.y;

    outputColor = vec4(diffuse + specular, 1.0);
    outputColor.rgb *= (ao * 0.5 + 0.5);
}
