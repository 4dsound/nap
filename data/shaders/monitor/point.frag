#version 330 core


uniform UBO {
    uniform vec3 color;
} ubo;

// output
out vec4 out_Color;

void main()
{
    out_Color = vec4(ubo.color.r, ubo.color.g, ubo.color.b, 1.);
}
