#version 150 core

out vec4 out_Color;
in vec3 pass_Position;

void main()
{
	float distance = length(pass_Position - vec3(0, 0, 0));
	vec3 color = vec3(1, 1, 1) * (0.3 - distance);
	out_Color = vec4(color, 1);
}
