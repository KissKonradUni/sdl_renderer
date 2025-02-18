#version 460 core

in vec3 fragmentPosition;
in vec3 fragmentNormal;
in vec3 fragmentTangent;
in vec2 fragmentUV;
in mat3 tangentSpaceMatrix;

in vec3 cameraPosition;
in vec3 cameraDirection;

out vec4 outputColor;

uniform sampler2D textureDiffuse;
uniform sampler2D textureNormal;
uniform sampler2D textureRoughness;

vec3 normalWithMap()
{
    vec3 tangentNormal = texture(textureNormal, fragmentUV).rgb;
    tangentNormal = tangentNormal * 2.0 - 1.0;
    tangentNormal = normalize(tangentSpaceMatrix * tangentNormal);
    
    return normalize(tangentNormal);
}

void main()
{
    vec3 normal = normalWithMap();

    vec3 lightPos   = vec3(0.0, 0.0, 2.0);

    vec3 lightDir   = normalize(lightPos - fragmentPosition);
    vec3 viewDir    = normalize(cameraPosition - fragmentPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = texture(textureDiffuse, fragmentUV).rgb * diff;

    float roughness = texture(textureRoughness, fragmentUV).r;
    float specPower = mix(16.0, 256.0, 1.0 - roughness);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), specPower);
    vec3 specular = vec3(0.5) * spec;

    outputColor = vec4(diffuse + specular, 1.0);
}