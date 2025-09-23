#version 460 core

layout (location = 0) in vec3 vertexPosition;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

void main()
{
    // Reverse order for column-major matrix
    gl_Position = vec4(vertexPosition, 1.0) * modelMatrix * viewMatrix * projectionMatrix;
}