#version 450 core
uniform nap
{
    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 modelMatrix;
} mvp;

uniform UBOVert
{
    uniform float level;
} ubovert;

in vec3 in_Position;
in vec3	in_UV0;

out vec3 pass_UV0;
out float pass_Level;

void main(void)
{
    pass_UV0 = in_UV0;
    pass_Level = ubovert.level;
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
}
