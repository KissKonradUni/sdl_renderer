#version 460 core

in vec3 fragmentPosition;
in vec3 fragmentNormal;
in vec2 fragmentUV;

in vec3 cameraPosition;
in vec3 cameraDirection;

out vec4 outputColor;

uniform sampler2D textureDiffuse;

void main()
{
    vec3 normal = normalize(fragmentNormal);
    vec3 lightDirection = normalize(vec3(1.0, 1.0, 1.0));

    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = texture(textureDiffuse, fragmentUV).rgb * diff;

    vec3 lightDir   = normalize(cameraPosition - fragmentPosition);
    vec3 viewDir    = normalize(cameraDirection);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    outputColor = vec4(vec3(0.1) + diffuse + spec, 1.0);
}