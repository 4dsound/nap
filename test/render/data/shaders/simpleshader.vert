#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding=0) uniform UniformBufferObject
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
}matBuf;

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec3 in_Color0;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(matBuf.projectionMatrix * matBuf.viewMatrix * matBuf.modelMatrix * vec3(in_Position.xy, 1.0), 0.0, 1.0);
    fragColor = in_Color0;
}