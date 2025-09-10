#version 460 core

in vec2 fragmentUV;

out vec4 outputColor;

uniform sampler2D gDiffuse;
uniform sampler2D gNormal;
uniform sampler2D gPosition;
uniform sampler2D gAORoughnessMetallic;

in vec3 cameraPosition;
in vec3 cameraDirection;

void main()
{
    vec3 albedo = texture(gDiffuse, fragmentUV).rgb;
    vec3 normal = normalize(texture(gNormal, fragmentUV).rgb * 2.0 - 1.0);
    vec3 position = texture(gPosition, fragmentUV).rgb;
    vec4 aoRoughnessMetallic = texture(gAORoughnessMetallic, fragmentUV);

    float ao        = aoRoughnessMetallic.r;
    float roughness = aoRoughnessMetallic.g;
    float metallic  = aoRoughnessMetallic.b;

    roughness = max(roughness, metallic);

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

    outputColor = vec4(diffuse + specular, 1.0);
    outputColor.rgb *= (ao * 0.5 + 0.5);
}
