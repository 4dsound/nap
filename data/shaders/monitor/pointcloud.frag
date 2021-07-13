#version 150 core

uniform UBO
{
	uniform vec3 color;
} ubo;

out vec3 out_Color;

void main()
{
	out_Color = ubo.color;
}