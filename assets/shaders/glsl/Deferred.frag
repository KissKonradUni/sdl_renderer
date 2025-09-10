#version 460 core

layout (location = 0) out vec4 gDiffuse;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gPosition;
layout (location = 3) out vec4 gAORoughnessMetallic;

in vec3 fragmentPosition;
in vec3 fragmentNormal;
in vec3 fragmentTangent;
in vec2 fragmentUV;
in mat3 tangentSpaceMatrix;

in vec3 cameraPosition;
in vec3 cameraDirection;

uniform sampler2D textureDiffuse;
uniform sampler2D textureNormal;
uniform sampler2D textureAORoughnessMetallic;

vec3 normalWithMap()
{
    vec3 tangentNormal = texture(textureNormal, fragmentUV).rgb;
    tangentNormal = tangentNormal * 2.0 - 1.0;
    
    vec3 worldNormal = tangentSpaceMatrix * tangentNormal;

    return normalize(worldNormal);
}

void main()
{
    vec4 combinedData = texture(textureAORoughnessMetallic, fragmentUV);
    float ao        = combinedData.r;
    float roughness = combinedData.g;
    float metallic  = combinedData.b;

    vec3 normal = normalWithMap();
    vec3 diffuse = texture(textureDiffuse, fragmentUV).rgb;

    gDiffuse = vec4(diffuse, 1.0);
    gNormal = vec4(normal * 0.5 + 0.5, 1.0);
    gPosition = vec4(fragmentPosition, 1.0);
    gAORoughnessMetallic = vec4(ao, roughness, metallic, 1.0);
}