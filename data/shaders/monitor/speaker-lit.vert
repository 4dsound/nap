#version 150 core

// NAP uniform buffer objects, contains all the matrices
uniform nap
{
	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	uniform mat4 modelMatrix;
} mvp;

uniform UBOVert {
	uniform float u_scale;
} ubovert;

in vec3	in_Position;
in vec3	in_Normal;

out vec3 pass_Position;
out vec3 pass_Normal;

void main(void)
{
	pass_Position = (mvp.modelMatrix * vec4(in_Position * ubovert.u_scale, 1.0)).xyz;
	pass_Normal = normalize(mvp.modelMatrix * vec4(in_Normal, 0.0)).xyz;
	
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * vec4(pass_Position, 1.0);
}
