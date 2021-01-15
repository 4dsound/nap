#version 150 core

in float pass_show;
in float pass_alpha;
in float pass_cameraDistance;

uniform UBO
{
	uniform vec3 color;
} ubo;

out vec4 out_Color;

void main()
{
	out_Color = vec4(ubo.color, pass_alpha) * pass_show;
}