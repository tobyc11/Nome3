#version 330 core

layout(location = 0) in vec3 LocalPos;
layout(location = 1) in vec2 ScaleRot;
layout(location = 2) in vec3 Origin;

out vec3 worldPos;

// Same as SolidMesh
uniform mat4 MMat;
uniform mat4 VPMat;

void main()
{
    vec3 scaledPos = ScaleRot.x * LocalPos;
    float theta = ScaleRot.y;
    mat2 xzMat = mat2(cos(theta), -sin(theta), sin(theta), cos(theta));
    scaledPos.xz = xzMat * scaledPos.xz;
    vec3 inPosition = Origin + scaledPos;

    // Same as SolidMesh
    vec4 worldPosPre = MMat * vec4(inPosition, 1.0);
    worldPos = worldPosPre.xyz / worldPosPre.w;
    gl_Position = VPMat * worldPosPre;
}
