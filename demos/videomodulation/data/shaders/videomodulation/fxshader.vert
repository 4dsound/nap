#version 450 core

uniform nap
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
} mvp;

// Input Vertex Attributes
in vec3	in_Position;
in vec3 in_UV0;

// Output to fragment shader
out vec3 passUVs;					//< vetex uv's
out vec3 passPosition;				//< vertex world space position

// uniform inputs
layout(binding = 10) uniform sampler2D	videoTexture;

void main(void)
{
	// Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);

	// Forward uvs to fragment shader
	passUVs = in_UV0;
}