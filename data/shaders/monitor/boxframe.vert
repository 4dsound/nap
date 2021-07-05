// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform nap
{
	// Vertex shader uniforms
	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	uniform mat4 modelMatrix;
} mvp;

uniform UBOVert
{
	uniform vec3 scale;
} ubovert;

// Input attributes
in vec3	in_Position;		// Vertex position
in vec3 in_Normal;			// Vertex normal
in vec3 in_Displacement;	// Vertex displacement

// Output attributes to fragment shader
out mat4 pass_ModelMatrix;
out vec3 pass_Vert;
out vec3 pass_Normals;

void main(void)
{
	// Calculate position
	vec3 displacement = in_Displacement * 0.05 / vec3(max(1, ubovert.scale.x), max(1, ubovert.scale.y), max(1, ubovert.scale.z));
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position + displacement, 1.0);

	pass_Vert = in_Position + in_Displacement * 0.1;
	pass_ModelMatrix = mvp.modelMatrix;
	pass_Normals = in_Normal;
}
