#version 410 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

out vec3 worldNormal;
out vec3 worldPos;

uniform mat4 MMat;
uniform mat4 VPMat;
uniform mat3 NormalMat;

void main()
{
    worldNormal = NormalMat * inNormal;
    vec4 worldPosPre = MMat * vec4(inPosition, 1.0);
    worldPos = worldPosPre.xyz / worldPosPre.w;
    gl_Position = VPMat * worldPosPre;
}
