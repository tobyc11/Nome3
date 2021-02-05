#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;
in int colorSelected;  // Added on 12/3 0 = not selected. 1 = selected
in vec3 faceColor; // Added on 12/2. faceColor = 999 999 999 by default. In that case, just use kd

out EyeSpaceVertex {
    vec3 position;
    vec3 normal;
    int colorSelected; // Randy added this on 12/3
    vec3 faceColor;
} vs_out;



uniform mat4 modelView;
uniform mat3 modelViewNormal;
uniform mat4 mvp;

void main()
{
    vs_out.normal = normalize( modelViewNormal * vertexNormal );
    vs_out.position = vec3( modelView * vec4( vertexPosition, 1.0 ) );

    vs_out.colorSelected = colorSelected; // Randy added this on 12/3
    vs_out.faceColor = faceColor; // Randy added this on 12/12
    gl_Position = mvp * vec4( vertexPosition, 1.0 );


}
