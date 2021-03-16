#version 150 core

out vec4 out_Color;
in vec3 pass_Position;

void main()
{
	float distance = length(pass_Position - vec3(0, 0, 0));
	out_Color = vec4(1.0, 1.0, 1.0, 0.3 - distance);
}
