#version 410 core

in vec3 worldNormal;
in vec3 worldPos;

out vec4 fragColor;

layout(std140) uniform LightingEnv
{
    vec3 CameraPos;
    vec3 LightPos;
};

void main()
{
    vec3 N = normalize(worldNormal);
    vec3 L = normalize(LightPos - worldPos);
    vec3 V = normalize(CameraPos - worldPos);
    vec3 H = normalize(L + V);
    float NoL = dot(N, L);
    if (NoL < 0.0f) NoL = -NoL;

    // Specular lighting based on half vector
    float NoH = dot(N, H);
    if (NoL < 0.0f) NoL = -NoL;
    float Alpha_spec = 64.0f;
    float Ispec = pow(NoH, Alpha_spec);

    vec3 Kdiff = vec3(0.8f, 0.8f, 1.0f);
    vec3 Kspec = vec3(1.0f, 1.0f, 1.0f);

    fragColor = vec4(Kdiff * NoL + Kspec * Ispec, 1.0);
}
