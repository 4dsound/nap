// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

in vec3 pass_Normals;		// Normals
in mat4 pass_ModelMatrix;	// Matrix
in vec3 pass_Vert;			// The vertex position

in float pass_Selected;    // Selected.

out vec4 out_Color;

uniform UBO
{
	uniform vec3		lightPositions[3];	// World position of the lights
	uniform int			lightCount;			// Number of lights
	uniform float		lightIntensity;		// Light intensity
	uniform float		ambientIntensity;	// Ambient light intensity
	uniform float		shininess;			// Specular angle shininess
	uniform float		specularIntensity;	// Amount of added specular
	uniform float		attenuationScale;	// Light Falloff
	uniform vec3		cameraLocation;		// World Space location of the camera
	uniform vec3        color;
    uniform vec3        depthColor;         // Color that particles fade to when disappearing in the back.
    uniform float       depthThreshold;     // Threshold of fading to grey.
    uniform float       depthCurvature;     // Power of fading.
} ubo;



void main(void)
{

	//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(pass_ModelMatrix)));
    vec3 normal = normalize(normal_matrix * pass_Normals);
	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = vec3(pass_ModelMatrix * vec4(pass_Vert, 1));
	// calculate vector that defines the distance from camera to the surface
	vec3 cameraPosition = ubo.cameraLocation;

    
    // depth buffer blend
    vec3 linearColor = ubo.color.rgb;
    float depthThreshold = 10.;
    float depthValue = 0.f;
    vec3 depthColor = mix(ubo.depthColor, linearColor, 0.4);
    if(pass_Selected < 0.f)
        depthValue = pow(min(1.0, length(ubo.cameraLocation - frag_position) / ubo.depthThreshold), ubo.depthCurvature);
    vec3 finalColor = depthValue * depthColor + (1. - depthValue) * linearColor;

    // if selected, display in white.
    if(pass_Selected > 0.f)
        finalColor = vec3(1, 1, 1);
    
	//final color (after gamma correction)
	out_Color = vec4(finalColor, 1);
}
