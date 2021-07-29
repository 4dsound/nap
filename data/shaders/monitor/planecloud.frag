#version 450 core

uniform UBO
{
    uniform vec3 color;
} ubo;

in vec3 pass_UV0;
in vec3 pass_Position;
in vec3 pass_CameraPosition;
in vec3 pass_Scale;

// in vec2 pass_Direction;


in float pass_DryWet;
out vec4 out_Color;


void main()
{
    out_Color = vec4(ubo.color, 0.5);

    // -- V1 --
    float distanceToCenter = distance(pass_UV0, vec3(0.5, 0.5, 0));
    float xDistance = abs(pass_UV0.x - 0.5);
    float yDistance = abs(pass_UV0.y - 0.5);
    float distanceToCamera = distance(pass_Position, pass_CameraPosition);
    float scaleFactor = (pass_Scale.x + pass_Scale.y + pass_Scale.z)/3.0;
    float alpha = max(0, 0.5 - distanceToCenter) * 0.3 * 1.0 - xDistance;
    alpha *= pow(1.0 - yDistance, 0.5);

    float cameraDistanceMultiplier = 0.5 + distance(pass_CameraPosition, pass_Position) / 50.f;
    alpha *= cameraDistanceMultiplier;

    alpha *= 2.;

    vec3 color = mix(vec3(0.2, 0.2, 0.2), ubo.color, pass_DryWet);
    out_Color = vec4(color, alpha);


    // -- V2 --
    // float xLineMultiplier = 0.;
    // if(abs(pass_UV0.x - 0.5) < 0.025)
    // 	xLineMultiplier = 1.;
    // float yMultiplier = 0.5 - abs(pass_UV0.y - 0.5);
    // vec3 color = mix(vec3(0.2, 0.2, 0.2), ubo.color, pass_DryWet);
    // float distanceMultiplier = distance(pass_CameraPosition, pass_Position) / 50.f;
    // out_Color = vec4(color, xLineMultiplier * yMultiplier * distanceMultiplier * 0.1);


    // -- V3 --
    // float distanceValue = 0.;
	// float distanceToCenter = distance(pass_UV0, vec3(0.5, 0.5, 0));
	// if(distanceToCenter > 0.25)
	// 	distanceValue = 0.;
	// else
	// 	distanceValue = 1.;
	// vec3 color = mix(vec3(0.2, 0.2, 0.2), ubo.color, pass_DryWet);
    // out_Color = vec4(color, 0.05 * distanceValue);


}
