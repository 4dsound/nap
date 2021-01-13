#version 150 core

in float pass_show;
in float pass_alpha;
in float pass_cameraDistance;

uniform UBO
{
	uniform vec3 color;
} ubo;

out vec4 out_Color;

void main()
{
	float maxCameraDistance = 7;
	float distanceFade =  1 - 0.5 * (pass_cameraDistance / maxCameraDistance);
	vec3 fadedColor = mix(vec3(0.1, 0.1, 0.1), ubo.color, distanceFade);
	out_Color = vec4(fadedColor, pass_alpha) * pass_show;
}