#version 460 core

out vec4 outputColor;

uniform sampler2D gDiffuse;
uniform sampler2D gNormal;
uniform sampler2D gPosition;
uniform sampler2D gAORoughnessMetallic;

// xy = screen size, zw = texture size
uniform vec4 screenAndTextureSize;

in vec2 fragmentUV;

in vec3 cameraPosition;
in vec3 cameraDirection;
in mat4 cameraMatrix;

void main()
{
    vec3 albedo = texture(gDiffuse, fragmentUV).rgb;
    vec3 normal = normalize(texture(gNormal, fragmentUV).rgb * 2.0 - 1.0);
    vec3 position = texture(gPosition, fragmentUV).rgb;
    vec4 aoRoughnessMetallic = texture(gAORoughnessMetallic, fragmentUV);

    float ao        = aoRoughnessMetallic.r;
    float roughness = aoRoughnessMetallic.g;
    float metallic  = aoRoughnessMetallic.b;

    outputColor = vec4(fragmentUV.xy, 0.0, 1.0);
}
