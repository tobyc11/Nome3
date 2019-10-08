#version 330 core

in vec3 vertexPosition;
in vec3 vertexColor;

out IOInterface
{
    vec3 vertexColor;
} outData;

uniform mat4 mvp;

void main()
{
    outData.vertexColor = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
