#version 460 core

in vec2 fragmentUV;

out vec4 outputColor;

uniform sampler2D skyboxTexture;

void main()
{
    outputColor = vec4(texture(skyboxTexture, fragmentUV).rgb, 1.0);
}
