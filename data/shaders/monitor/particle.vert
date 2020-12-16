#version 330 core

// NAP uniform buffer objects, contains all the matrices
uniform nap
{
    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 modelMatrix;
} mvp;

uniform UBOVert {
    uniform float useColorFromUniform;
    uniform vec4 color;
} ubovert;

// Input Vertex Attributes
in vec3 in_Position;
in vec3 in_Normal;
in vec4 in_Color0;

// Output to fragment shader
out vec3 passNormal;                //< vertex normal in world space
out vec3 passPosition;                //< vertex world space position
out vec4 passColor;                    //< vertex color

void main(void)
{
    // Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
    
    //rotate normal based on model matrix and set
    mat3 normal_matrix = mat3(mvp.modelMatrix);
    passNormal = normalize(normal_matrix * in_Normal);
    
    // calculate vertex world space position and set
    passPosition = vec3(mvp.modelMatrix * vec4(in_Position, 1));
    
    // Pass color attribute
    passColor = ubovert.useColorFromUniform != 0.0 ? ubovert.color : in_Color0;
}