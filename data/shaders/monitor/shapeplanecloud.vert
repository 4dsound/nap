#version 450 core

uniform nap
{
    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 modelMatrix;
} mvp;

uniform UBOVert
{
    uniform vec3 scale; // Sound object scale
    uniform vec2 renderTargetSize;
    uniform float time;
    uniform vec3 cameraPosition;
    uniform int spatialDelay_enable;
    uniform float spatialDelay_dryWet;
    uniform float spatialDelay_peripheralScale;
    uniform float spatialDelay_random;
    uniform float spatialDelay_noiseDepth;
    uniform float spatialDelay_noiseSpeed;
    uniform float spatialDelay_smooth;
    uniform float spatialDelay_modulationTimePassed;
    uniform float spatialDelay_feedback;
} ubovert;


in vec3 in_Position; // Point center position
in vec3 in_Normal;
in vec3 in_RelativePosition; // Position of the plane vertex relative to the point center
in vec3	in_UV0; // UV on the point plane
in vec3 in_UV1; // UV on the shape surface
in int in_Index; // Vertex index

out vec3 pass_UV0;
out vec3 pass_Position;
out vec3 pass_CameraPosition;
out vec3 pass_Scale;

out vec2 pass_Direction;

out float pass_DryWet;

out float pass_Intensity;

// By Inigo Quilez.
vec2 grad(ivec2 z)
{
    // 2D to 1D  (feel free to replace by some other)
    int n = z.x+z.y*11111;

    // Hugo Elias hash (feel free to replace by another one)
    n = (n<<13)^n;
    n = (n*(n*n*15731+789221)+1376312589)>>16;

    // simple random vectors
    return vec2(cos(float(n)),sin(float(n)));
}

// By Inigo Quilez.
float noise( in vec2 p )
{
    ivec2 i = ivec2(floor( p ));
    vec2 f = fract( p );
    vec2 u = f*f*(3.0-2.0*f);
    return mix( mix( dot( grad( i+ivec2(0,0) ), f-vec2(0.0,0.0) ),
    dot( grad( i+ivec2(1,0) ), f-vec2(1.0,0.0) ), u.x),
    mix( dot( grad( i+ivec2(0,1) ), f-vec2(0.0,1.0) ),
    dot( grad( i+ivec2(1,1) ), f-vec2(1.0,1.0) ), u.x), u.y);
}


void main(void)
{
    // Pass UV coordinates to frag shader
    pass_UV0 = in_UV0;
    // Pass point position to frag shader
    pass_Position = in_Position;
    // Pass camera position to frag shader
    pass_CameraPosition = ubovert.cameraPosition;
    // Pass sound object scale to frag shader
    pass_Scale = ubovert.scale;

    pass_DryWet = ubovert.spatialDelay_dryWet;

    float spatialDelayAmount = ubovert.spatialDelay_enable;

    // Modulate the input position for the feedback effect
    float feedbackRand = (in_Index % 100)/100.0 * (ubovert.spatialDelay_feedback * 0.8);
    float randRounded = int(feedbackRand * 5) / 5.0;
    float feedbackMultiplier =  (1. - randRounded) * (1. + 0.5 * ubovert.spatialDelay_feedback);

    // Peripheral multiplier
    float peripheralProximity = distance(vec3(0, 0, 0), 0.5f * ubovert.scale) - distance(vec3(0, 0, 0), in_Position);
    float peripheralMultiplier = 1. + ubovert.spatialDelay_peripheralScale * 0.04;

    // Random noise displacement
	float modulationAmplitude = ubovert.spatialDelay_noiseDepth * ubovert.spatialDelay_noiseDepth * 2.;
	float modulationNoise = 0.5 + 1.5 * noise(vec2(in_UV1.x * 5 + in_UV1.z * 2 + ubovert.spatialDelay_modulationTimePassed / 4., in_UV1.y * 5 + in_UV1.z * 2)) * max(0, 1.0 - distance(vec3(in_UV1.x, in_UV1.y, 0), vec3(0.5, 0.5, 0)));
	float modulationMultiplier = modulationNoise * modulationAmplitude;


    // Calculate final point position
	vec3 position = in_Position * feedbackMultiplier * peripheralMultiplier + in_Position * modulationMultiplier;  

    // Calculate the point plane size
    float aspectRatio = ubovert.renderTargetSize.y / ubovert.renderTargetSize.x;
    vec3 relativePosition = vec3(in_RelativePosition.x * aspectRatio * 0.1, in_RelativePosition.y, 0);

	// create new model matrix that only takes into account the translation.
    mat4 matrix;
    matrix[0] = vec4(1.0, 0.0, 0.0, 0.0); // first column
    matrix[1] = vec4(0.0, 1.0, 0.0, 0.0); // second column
    matrix[2] = vec4(0.0, 0.0, 1.0, 0.0); // third column
    matrix[3] = vec4(mvp.modelMatrix[3][0], mvp.modelMatrix[3][1], mvp.modelMatrix[3][2], 1.0); // fourth column


    // calculate scaling factor
    float scaleAverage = (ubovert.scale.x + ubovert.scale.y + ubovert.scale.y) / 3.;
    scaleAverage *= 0.66;
    float boundScale = 1. + scaleAverage * 0.3;


	// correctly position the SphereMesh around the sound object transform box
	vec3 postScalePosition = boundScale * position;

    // calculate gl_Position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * matrix * vec4(postScalePosition, 1) + vec4(relativePosition * 1. * boundScale, 0);
}
