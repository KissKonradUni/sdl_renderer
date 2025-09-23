#version 460 core

out float outputDepth;

void main()
{
    outputDepth = gl_FragCoord.z;
}
