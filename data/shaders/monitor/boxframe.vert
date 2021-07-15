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
    uniform float selected;
} ubovert;

// Input attributes
in vec3	in_Position;		// Vertex position
in vec3 in_Normal;			// Vertex normal
in vec3 in_Displacement;	// Vertex displacement

// Output attributes to fragment shader
out mat4 pass_ModelMatrix;
out vec3 pass_Vert;
out vec3 pass_Normals;
out float pass_Selected;

void main(void)
{


    // ensure minimum scale, don't show if the particle will be visible as a point particle.
    mat4 modelMatrix = mvp.modelMatrix;
    if(ubovert.scale.x < 0.1f && ubovert.scale.y < 0.1f && ubovert.scale.z < 0.1f)
    {
        modelMatrix[0][0] = 0.0;
        modelMatrix[1][1] = 0.0;
        modelMatrix[2][2] = 0.0;
    }
    else
    {
        if(ubovert.scale.x < 0.1f)
            modelMatrix[0][0] = 0.1f;
        if(ubovert.scale.y < 0.1f)
            modelMatrix[1][1] = 0.1f;
        if(ubovert.scale.z < 0.1f)
            modelMatrix[2][2] = 0.1f;
    }

    float selectedMultiplier = 1.f;
    if(ubovert.selected >= 0.f)
        selectedMultiplier = 2.f;


    vec3 displacement = in_Displacement * selectedMultiplier * 0.02f;
    displacement = vec3(vec4(displacement, 1.) * inverse(modelMatrix));
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * modelMatrix * vec4(in_Position + displacement, 1.0);

	pass_Vert = in_Position + in_Displacement;
	pass_ModelMatrix = mvp.modelMatrix;
	pass_Normals = in_Normal;
	pass_Selected = ubovert.selected;
}
