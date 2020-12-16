#version 330 core

// NAP uniform buffer objects, contains all the matrices
uniform nap
{
    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 modelMatrix;
} mvp;

in vec3	in_Position;
in vec2 in_UV0;

out vec2 pass_uv;

void main(void)
{
	// Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);

	// Forward uvs to fragment shader
	pass_uv = in_UV0.xy;
}