#version 450 core

uniform UBO
{
    uniform vec3 color;
} ubo;

in vec3 pass_UV0;
out vec3 out_Color;

void main()
{
//    float distanceToCenter = distance(pass_UV0, vec3(0.5, 0.5, 0));
//    if (distanceToCenter > 0.5)
//        alpha = 0;
    out_Color = vec3(ubo.color);
}
