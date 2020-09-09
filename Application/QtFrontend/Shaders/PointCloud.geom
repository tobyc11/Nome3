#version 330 core

layout(points) in;
layout(max_vertices = 4, triangle_strip) out;

in vec3 fragmentColor[1];

out vec3 pgColor;
out vec2 pgUV;

uniform float pointRadius;
uniform vec3 pointAmbientColor;
uniform sampler2D pointTexture;

uniform float surfaceWidth;
uniform float surfaceHeight;

void main()
{
    vec4 xOffset = vec4(pointRadius / surfaceWidth * gl_in[0].gl_Position.w, 0, 0, 0);
    vec4 yOffset = vec4(0, pointRadius / surfaceHeight * gl_in[0].gl_Position.w, 0, 0);
    gl_Position = (gl_in[0].gl_Position + xOffset) - yOffset;
    pgColor = fragmentColor[0];
    pgUV = vec2(1.0, 0.0);
    EmitVertex();
    gl_Position = (gl_in[0].gl_Position + xOffset) + yOffset;
    pgColor = fragmentColor[0];
    pgUV = vec2(1.0);
    EmitVertex();
    gl_Position = (gl_in[0].gl_Position - xOffset) - yOffset;
    pgColor = fragmentColor[0];
    pgUV = vec2(0.0);
    EmitVertex();
    gl_Position = (gl_in[0].gl_Position - xOffset) + yOffset;
    pgColor = fragmentColor[0];
    pgUV = vec2(0.0, 1.0);
    EmitVertex();
    EndPrimitive();
}
