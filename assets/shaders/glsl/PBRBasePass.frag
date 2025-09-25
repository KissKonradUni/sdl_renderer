#version 460 core

in vec2 fragmentUV;

out vec4 outputColor;

uniform sampler2D gDiffuse;
uniform sampler2D gNormal;
uniform sampler2D gPosition;
uniform sampler2D gAORoughnessMetallic;

in vec3 cameraPosition;
in vec3 cameraDirection;
in mat4 cameraMatrix;

uniform sampler2D shadowMap;
uniform mat4 lightViewMatrix;
uniform mat4 lightProjectionMatrix;
uniform vec4 lightDirection;

uniform sampler2D skyboxTexture;

vec2 sampleEquirectangularMap(vec3 dir)
{
    float u = 0.5 + (atan(dir.z, dir.x) / (2.0 * 3.14159265359));
    float v = 0.5 - (asin(dir.y) / 3.14159265359);
    return vec2(u, 1.0 - v);
}

float checkShadow(vec3 fragPos, vec3 normal)
{
    vec4 lightSpacePos = vec4(fragPos, 1.0) * lightViewMatrix * lightProjectionMatrix;
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    // If outside shadow map, return no shadow
    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    float bias = 0.001;
    float shadow = 0.0;
    float samples = 0.0;
    float texelSize = 1.0 / textureSize(shadowMap, 0).x;

    // 3x3 PCF kernel
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
            samples += 1.0;
        }
    }
    shadow /= samples;
    return shadow;
}

// Direcitional light
vec2 calculateSkyboxLighting(vec3 position, vec3 normal, vec3 viewDir, float roughness)
{
    vec3 halfwayDir = normalize(lightDirection.xyz + viewDir);

    float diff = max(dot(normal, lightDirection.xyz), 0.0);
    float shininess = mix(256.0, 8.0, roughness); // Lower shininess for rougher surfaces
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

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

    if (length(position) <= 0.001)
    {
        discard;
    }

    vec3 viewDir = normalize(cameraPosition - position);
    vec3 reflectDir = reflect(-viewDir, normal);
    vec3 skyboxColor = texture(skyboxTexture, sampleEquirectangularMap(reflectDir)).rgb;

    float shadow = checkShadow(position, normal);
    vec2 skyboxLighting = calculateSkyboxLighting(position, normal, cameraDirection, roughness) * (1.0 - shadow * 0.80);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 specular = skyboxLighting.y * mix(F0, skyboxColor, metallic);
    vec3 diffuse = albedo * skyboxLighting.x;
    
    outputColor = vec4(diffuse + specular, 1.0);
    outputColor.rgb *= ao;
}
