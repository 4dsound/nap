#version 450 core
uniform UBO
{
    uniform vec3 color;
    uniform vec3 cameraPosition;
} ubo;

in vec3 pass_Position;
out vec4 out_Color;
void main()
{
    out_Color = vec4(ubo.color, 1.0);
}