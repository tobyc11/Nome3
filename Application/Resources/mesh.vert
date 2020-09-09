#version 330 core

in vec3 inPosition;

out VertexInterpolants
{
    vec3 Color;
} outData;

uniform mat4 Model;

uniform mat4 View;
uniform mat4 Proj;

void main()
{
    gl_Position = Proj * View * Model * vec4(inPosition, 1.0);
    outData.Color = vec3(1);
}
