#version 460 core

// This vertex shader is used to put the quad on the screen for the final pass
layout (location = 0) in vec2 vertexPosition;

out vec2 fragmentUV;

layout(std140, binding = 0) uniform Camera {
    mat4 camView;
    mat4 camProjection;
    vec3 camPosition;
    vec3 camDirection;
};

out vec3 cameraPosition;
out vec3 cameraDirection;

void main()
{
    gl_Position = vec4(vertexPosition, 0.0, 1.0);
    fragmentUV = (vertexPosition + 1.0) * 0.5;

    cameraPosition = camPosition;
    cameraDirection = camDirection;
}