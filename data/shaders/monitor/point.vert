#version 330 core

// NAP uniform buffer objects, contains all the matrices
uniform nap
{
    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 modelMatrix;
} mvp;

uniform UBOVert
{
    uniform vec3 scale;
} ubovert;

// Input Vertex Attributes
in vec3 in_Position;

void main(void)
{
    // Multiply by 0 if the dimensions are not 0.
    float multiplier = 0.;
    if(ubovert.scale.x < 0.251 && ubovert.scale.y < 0.251 && ubovert.scale.z < 0.251)
        multiplier = 1.;
    
    // Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0) * multiplier;
}
