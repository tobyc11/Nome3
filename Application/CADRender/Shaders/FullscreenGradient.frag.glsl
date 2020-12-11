#version 330 core

in vec2 QuadUV;

out vec4 FragColor;

uniform vec3 TopColor;
uniform vec3 BottomColor;

void main()
{
    float t = QuadUV.y;
    FragColor = (1 - t) * vec4(TopColor, 0) + t * vec4(BottomColor, 0);
}
