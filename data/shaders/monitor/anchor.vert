#version 450 core
uniform nap
{
    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 modelMatrix;
} mvp;

uniform UBOVert
{
    uniform vec3 positionOffset;
} ubovert;

in vec3 in_Position;
out vec3 pass_Position;

void main(void)
{
    pass_Position = in_Position + ubovert.positionOffset;
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * vec4(in_Position + ubovert.positionOffset, 1.0);
}
