#version 330 core

in vec3 inPosition;

out VertexData
{
    vec3 Color;
} outData;

uniform ConstantPerObject
{
    mat4 Model;
} cbPerObject;

uniform ConstantPerView
{
    mat4 View;
    mat4 Proj;
} cbPerView;

void main()
{
    gl_Position = cbPerView.Proj * cbPerView.View * cbPerObject.Model * vec4(inPosition, 1.0);
    outData.Color = vec3(1);
}
