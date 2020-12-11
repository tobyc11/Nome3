#version 330 core

out vec2 QuadUV;

void main()
{
    // 3-2
    // |/|
    // 0-1
    if (gl_VertexID == 0)
    {
        gl_Position = vec4(-1, -1, 1, 1);
        QuadUV = vec2(0, 1);
    }
    else if (gl_VertexID == 1)
    {
        gl_Position = vec4(1, -1, 1, 1);
        QuadUV = vec2(1, 1);
    }
    else if (gl_VertexID == 2)
    {
        gl_Position = vec4(1, 1, 1, 1);
        QuadUV = vec2(1, 0);
    }
    else if (gl_VertexID == 3)
    {
        gl_Position = vec4(-1, 1, 1, 1);
        QuadUV = vec2(0, 0);
    }
}
