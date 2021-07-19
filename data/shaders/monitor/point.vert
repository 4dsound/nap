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
    // Invisible if the dimensions are too high.
    float multiplier = 0.;
    if(ubovert.scale.x < 0.1 && ubovert.scale.y < 0.1 && ubovert.scale.z < 0.1)
        multiplier = 1.;
    
    // set scaling factor to static 0.1.
    mat4 modelMatrix = mvp.modelMatrix;
    modelMatrix[0][0] = 0.1;
    modelMatrix[1][1] = 0.1;
    modelMatrix[2][2] = 0.1;
    
    // Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * modelMatrix * vec4(in_Position, 1.0) * multiplier;
}
