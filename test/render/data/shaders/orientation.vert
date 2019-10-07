#version 450 core

layout(binding=0) uniform UniformBufferObject
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
}matBuf;

layout(location=0) in vec3	in_Position;
layout(location=1) in vec4	in_Color0;
layout(location=2) in vec3 	in_Normals;
layout(location=3) in vec3	in_UV0;

layout(location=0) out vec4 pass_Color;
layout(location=1) out vec3 pass_Uvs;

void main(void)
{
	// Calculate position
    gl_Position = matBuf.projectionMatrix * matBuf.viewMatrix * matBuf.modelMatrix * vec4(in_Position, 1.0);

	// Pass color and uv's 
	pass_Color = in_Color0;
	pass_Uvs = in_UV0;
}