#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
in vec3 fragmentColor;

out vec4 fragColor;

uniform vec3 baseColor;

void main()
{
    vec3 L = normalize(vec3(1.0, 2.0, 3.0));
    float Kd = clamp(dot(L, worldNormal), 0.5, 1.0);
    vec3 diffuseColor = baseColor * 0.5 + fragmentColor * 0.5;
    fragColor = vec4(diffuseColor * Kd, 1.0);
    //fragColor = vec4(worldNormal, 1.0);
}
