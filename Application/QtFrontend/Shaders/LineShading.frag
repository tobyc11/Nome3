#version 330 core

in IOInterface
{
    vec3 vertexColor;
} inData;

out vec4 fragColor;

uniform vec3 instanceColor;

void main()
{
    vec3 additive = (vec3(1.0) - inData.vertexColor) * instanceColor;
    fragColor = vec4(instanceColor, 1.0);
    ///fragColor = vec4(inData.vertexColor + additive, 1.0); 
    //fragColor = vec4(inData.vertexColor, 1.0);
}
