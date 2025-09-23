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

uniform sampler2D shadowMap;
uniform mat4 lightViewMatrix;
uniform mat4 lightProjectionMatrix;

float checkShadow(vec3 fragPos)
{
    vec4 lightSpacePos = vec4(fragPos, 1.0) * lightViewMatrix * lightProjectionMatrix;
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    // If outside shadow map, return no shadow
    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    float bias = 0.005;
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
vec2 calculateSkyboxLighting(vec3 position, vec3 normal, vec3 viewDir)
{
    vec3 lightRot = vec3(1.4, 0.3, 0.0);
    vec3 lightDir = vec3(sin(lightRot.y) * cos(lightRot.x), sin(lightRot.x), cos(lightRot.y) * cos(lightRot.x));
    lightDir = normalize(lightDir);
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

    float shadow = checkShadow(position);
    vec2 skyboxLighting = calculateSkyboxLighting(position, normal, viewDir) * (1.0 - shadow);
    diffuse += albedo * skyboxColor.xyz * skyboxLighting.x;
    specular += skyboxColor.xyz * skyboxLighting.y;

    outputColor = vec4(diffuse + specular, 1.0);
    outputColor.rgb *= (ao * 0.5 + 0.5);
}
