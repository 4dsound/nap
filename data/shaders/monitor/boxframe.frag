// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

in vec3 pass_Normals;		// Normals
in mat4 pass_ModelMatrix;	// Matrix
in vec3 pass_Vert;			// The vertex position
in float pass_BypassDepth;  // True when particle is selected

out vec4 out_Color;

uniform UBO
{
	uniform vec3		lightPositions[2];	// World position of the lights
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
	float lightIntensity = ubo.lightIntensity;
	float alpha = 1;

    vec3[2] lightPositions;
    
    // convert light positions to object space, so they move together with the sound object
    // TODO: not sure if this is correct!
    for(int i = 0; i < ubo.lightCount; i++)
    {
        lightPositions[i] = vec3(vec4(ubo.lightPositions[i], 1) * inverse(pass_ModelMatrix));
    }

	//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(pass_ModelMatrix)));
    vec3 normal = normalize(normal_matrix * pass_Normals);

	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = vec3(pass_ModelMatrix * vec4(pass_Vert, 1));

	// calculate vector that defines the distance from camera to the surface
	vec3 cameraPosition = ubo.cameraLocation;
	vec3 surfaceToCamera = normalize(cameraPosition - frag_position);

	// Ambient color
	vec3 ambient = ubo.color.rgb * lightIntensity * ubo.ambientIntensity;

	vec3 linearColor = ambient;

	for (int i = 0; i < ubo.lightCount; i++)
	{
		//calculate the vector from this pixels surface to the light source
		vec3 surfaceToLight = normalize(lightPositions[i] - frag_position);

		//diffuse
		float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
		vec3 diffuse = diffuseCoefficient * ubo.color.rgb * lightIntensity;

		//specular
		vec3 specularColor = vec3(1.0,1.0,1.0);
		float specularCoefficient = 0.0;
		if(diffuseCoefficient > 0.0)
		specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), ubo.shininess);
		vec3 specular = specularCoefficient * specularColor * lightIntensity * ubo.specularIntensity;

		//attenuation based on light distance
		float distanceToLight = length(lightPositions[i] - frag_position);
		float attenuation = 1.0 / (1.0 + ubo.attenuationScale * pow(distanceToLight, 2));

		//linear color (color before gamma correction)
		linearColor = linearColor + attenuation * (diffuse + specular);
	}
    
    // depth buffer blend
    float depthThreshold = 10.;
    float depthValue = 0.f;
    if(pass_BypassDepth < 0.f)
        depthValue = pow(min(1.0, length(cameraPosition - frag_position) / ubo.depthThreshold), ubo.depthCurvature);
    vec3 finalColor = depthValue * ubo.depthColor + (1. - depthValue) * linearColor;
    
    
	//final color (after gamma correction)
	out_Color = vec4(finalColor, alpha);
}
