#version 330 core

in vec3 inPosition;

out VertexInterpolants
{
    vec3 Color;
} outData;

#ifdef USE_CONSTANT_BUFFER
uniform ConstantPerObject
{
#endif
    mat4 Model;
#ifdef USE_CONSTANT_BUFFER
};
#endif

#ifdef USE_CONSTANT_BUFFER
uniform ConstantPerView
{
#endif
    mat4 View;
    mat4 Proj;
#ifdef USE_CONSTANT_BUFFER
};
#endif

void main()
{
    gl_Position = Proj * View * Model * vec4(inPosition, 1.0);
    outData.Color = vec3(1);
}
