#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform float u_scale;

in vec3	in_Position;
in vec3	in_Normal;

out vec3 pass_Position;
out vec3 pass_Normal;

void main(void)
{
	pass_Position = (modelMatrix * vec4(in_Position * u_scale, 1.0)).xyz;
	pass_Normal = normalize(modelMatrix * vec4(in_Normal, 0.0)).xyz;
	
    gl_Position = projectionMatrix * viewMatrix * vec4(pass_Position, 1.0);
}
