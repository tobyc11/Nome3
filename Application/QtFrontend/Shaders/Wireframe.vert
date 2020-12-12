#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;
in int colorSelected;  // Added on 12/3 0 = not selected. 1 = selected

out EyeSpaceVertex {
    vec3 position;
    vec3 normal;
    int colorSelected; // Randy added this on 12/3
} vs_out;



uniform mat4 modelView;
uniform mat3 modelViewNormal;
uniform mat4 mvp;

void main()
{
    vs_out.normal = normalize( modelViewNormal * vertexNormal );
    vs_out.position = vec3( modelView * vec4( vertexPosition, 1.0 ) );

    vs_out.colorSelected = colorSelected; // Randy added this on 12/3
    gl_Position = mvp * vec4( vertexPosition, 1.0 );

}
