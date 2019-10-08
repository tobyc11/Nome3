#version 330 core

in IOInterface
{
    vec3 vertexColor;
} inData;

out vec4 fragColor;

void main()
{
    fragColor = vec4(inData.vertexColor, 1.0);
}
