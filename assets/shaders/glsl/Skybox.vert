#version 460 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 3) in vec2 vertexUV;

out vec2 fragmentUV;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 cameraPosition;

void main()
{
    // Reverse order for column-major matrix
    gl_Position = vec4(vertexPosition + cameraPosition.xyz, 1.0) * viewMatrix * projectionMatrix;
    fragmentUV = vertexUV;
}