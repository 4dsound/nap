#version 330 core


uniform UBO {
    uniform vec3 color;
} ubo;

// output
out vec4 out_Color;

in float pass_Selected;

void main()
{
    if(pass_Selected > 0.f)
        out_Color = vec4(1, 1, 1, 1);
    else
        out_Color = vec4(ubo.color.r, ubo.color.g, ubo.color.b, 1.);
}
