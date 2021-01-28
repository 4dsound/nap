#version 450 core
uniform UBO
{
    uniform vec3 color;
} ubo;

in vec3 pass_UV0;
in float pass_Level;
out vec4 out_Color;

void main()
{
    float distanceToCenter = distance(pass_UV0, vec3(0.5, 0.5, 0));
    vec3 color = mix(ubo.color, vec3(1, 1, 1), pass_Level * (1 - distanceToCenter * 2));
    out_Color = vec4(color, 1 - distanceToCenter * 2);
}