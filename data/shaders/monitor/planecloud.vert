#version 450 core

uniform nap
{
    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 modelMatrix;
} mvp;

uniform UBOVert
{
    uniform vec3 cameraPosition;
} ubovert;


in vec3 in_Position;
in vec3 in_RelativePosition;
in vec3	in_UV0;

out vec3 pass_UV0;

void main(void)
{
    pass_UV0 = in_UV0;
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position + in_RelativePosition * 0.1, 1.0);
}
