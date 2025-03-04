#version 460 core
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUV;

out vec3 fragmentPosition;
out vec3 fragmentNormal;
out vec3 fragmentTangent;
out vec2 fragmentUV;
out mat3 tangentSpaceMatrix;

out vec3 cameraPosition;
out vec3 cameraDirection;

layout(std140, binding = 0) uniform Camera {
    mat4 camView;
    mat4 camProjection;
    vec3 camPosition;
    vec3 camDirection;
};
uniform mat4 modelMatrix;

void main()
{
    // Reverse order for column-major matrix
    gl_Position = vec4(vertexPosition, 1.0) * modelMatrix * camView * camProjection;

    fragmentPosition = vec3(vec4(vertexPosition, 1.0) * modelMatrix);
    fragmentNormal = (vec4(vertexNormal, 1.0) * transpose(inverse(modelMatrix))).xyz;
    fragmentTangent = (vec4(vertexTangent, 1.0) * transpose(inverse(modelMatrix))).xyz;
    fragmentUV = vertexUV;

    vec3 bitangent = cross(fragmentNormal, fragmentTangent);
    tangentSpaceMatrix = mat3(fragmentTangent, bitangent, fragmentNormal);

    cameraPosition = camPosition;
    cameraDirection = camDirection;
}