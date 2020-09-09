#version 330 core

in vec3 pgColor;
in vec2 pgUV;

out vec4 fragColor;

uniform float pointRadius;
uniform vec3 pointAmbientColor;
uniform sampler2D pointTexture;

void main()
{
    vec4 baseColor = texture(pointTexture, pgUV).a * vec4(pointAmbientColor, 1);
    fragColor = baseColor + vec4(pgColor, 0);
}
