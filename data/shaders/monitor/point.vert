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
    if(ubovert.scale.x < 0.101 && ubovert.scale.y < 0.101 && ubovert.scale.z < 0.101)
        multiplier = 1.;
    
    // Create a matrix with a static scale of 0.1, taking the translation from the model matrix.
    mat4 matrix;
    matrix[0] = vec4(0.1, 0.0, 0.0, 0.0); // first column
    matrix[1] = vec4(0.0, 0.1, 0.0, 0.0); // second column
    matrix[2] = vec4(0.0, 0.0, 0.1, 0.0); // third column
    matrix[3] = vec4(mvp.modelMatrix[3][0], mvp.modelMatrix[3][1], mvp.modelMatrix[3][2], 1.0); // fourth column
    
    // Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * matrix * vec4(in_Position, 1.0) * multiplier;
}
