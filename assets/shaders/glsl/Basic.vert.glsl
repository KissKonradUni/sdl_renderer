#version 460 core
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;

out vec3 fragmentPosition;
out vec3 fragmentNormal;
out vec3 cameraPosition;
out vec3 cameraDirection;

uniform struct Camera {
    mat4 view;
    mat4 projection;
} camera;
uniform mat4 model;

void main()
{
    // Reverse order for column-major matrix
    gl_Position = vec4(vertexPosition, 1.0) * model * camera.view * camera.projection;

    fragmentPosition = vec3(vec4(vertexPosition, 1.0) * model);
    fragmentNormal = (vec4(vertexNormal, 1.0) * transpose(inverse(model))).xyz;

    cameraPosition = (inverse(camera.view))[3].xyz;
    cameraDirection = (inverse(camera.view) * vec4(0.0, 0.0, -1.0, 0.0)).xyz;
}