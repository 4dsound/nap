#version 450 core

uniform UBO
{
    uniform vec3 color;
} ubo;

in vec3 pass_UV0;
out vec4 out_Color;

void main()
{
//    out_Color = vec4(ubo.color, 0);
    float distanceToCenter = distance(pass_UV0, vec3(0.5, 0.5, 0));
    out_Color = vec4(ubo.color, 0.01);
}
