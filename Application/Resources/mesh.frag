#version 330 core

in VertexInterpolants
{
    vec3 Color;
} inData;

out vec4 pixelColor;

void main()
{
    pixelColor = vec4(inData.Color, 1);
}
