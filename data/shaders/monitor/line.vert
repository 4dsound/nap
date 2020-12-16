#version 150 core

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

in vec3	in_Position;
in vec4	in_Color0;

out vec4 pass_Color;

void main(void)
{
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);

    pass_Color = ubovert.useColorFromUniform != 0.0 ? ubovert.color : in_Color0;
}