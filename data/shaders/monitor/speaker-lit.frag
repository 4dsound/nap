#version 150 core

uniform vec4 u_color;

in vec3 pass_Position;
in vec3 pass_Normal;

out vec4 out_Color;

void main()
{
	float matSpecularExp = 20.0;

	vec3 totalLightIntensity = vec3(0.0);

	// accumulate directional light
	{
		vec3 lightDirection = normalize(vec3(1.0, 0.0, 1.0));
		vec3 lightIntensity = vec3(0.6, 0.7, 0.8);
		totalLightIntensity += pow(max(0.0, dot(pass_Normal, lightDirection)), matSpecularExp) * lightIntensity;
	}

	// accumulate spot light
#if 1
	{
		vec3 lightPosition = vec3(0.0, 0.0, 0.0);
		vec3 lightDelta = lightPosition - pass_Position;
		float lightDistance = length(lightDelta);
		vec3 lightDirection = normalize(lightDelta);
		vec3 lightIntensity = 10.0 * vec3(0.8, 0.7, 0.6);

		lightIntensity /= lightDistance * lightDistance + 0.01;

		totalLightIntensity += pow(max(0.0, dot(pass_Normal, lightDirection)), matSpecularExp) * lightIntensity;
	}
#endif
	
	// accumulate ambient
	{
		totalLightIntensity += vec3(0.02);
	}

	// go from linear to gamma space
	vec3 color = pow(totalLightIntensity, vec3(1.0 / 2.2));

	color *= u_color.rgb;

	out_Color = vec4(color, 1.0);
}
