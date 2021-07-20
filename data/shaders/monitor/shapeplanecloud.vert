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


// From https://github.com/glslify/glsl-look-at/blob/gh-pages/index.glsl
mat3 calcLookAtMatrix(vec3 origin, vec3 target, float roll) {
  vec3 rr = vec3(sin(roll), cos(roll), 0.0);
  vec3 ww = normalize(target - origin);
  vec3 uu = normalize(cross(ww, rr));
  vec3 vv = normalize(cross(uu, ww));

  return mat3(uu, vv, ww);
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


    // // calculate noise displacement for the spatiald delay random parameter
    // float randomAmplitude = pow(ubovert.spatialDelay_random/30.0, 0.3) * 3;
    // float randomDetail = 2;
    // vec3 randomDisplacement = in_Normal * randomAmplitude * noise(vec2(in_UV1.x * randomDetail + 1, in_UV1.y * randomDetail + 1)) * max(0, 0.6 - distance(vec3(in_UV1.x, in_UV1.y, 0), vec3(0.5, 0.5, 0)));

    // // calculate noise displacement for the noise modulation
    // float modulationAmplitude = pow(ubovert.spatialDelay_noiseDepth/1000, 0.3) * 6;
    // float modulationDetail = 2 + ubovert.spatialDelay_noiseSpeed * (1 - ubovert.spatialDelay_smooth) * 2;
    // float modulationNoise = noise(vec2(in_UV1.x * modulationDetail + ubovert.spatialDelay_modulationTimePassed, in_UV1.y * modulationDetail));
    // vec3 modulationDisplacement = in_Normal * modulationAmplitude * modulationNoise * max(0, 0.6 - distance(vec3(in_UV1.x, in_UV1.y, 0), vec3(0.5, 0.5, 0)));

    // // // Calculate the relative proximity to the sound object center
    // float peripheralProximity = distance(vec3(0, 0, 0), 0.5f * ubovert.scale) - distance(vec3(0, 0, 0), in_Position);
    // float peripheralScale = ubovert.spatialDelay_peripheralScale * 0.07;

    // Modulate the input position for the feedback effect
    float feedbackRand = (in_Index % 100)/100.0 * (ubovert.spatialDelay_feedback * 0.8);
    float randRounded = int(feedbackRand * 5) / 5.0;
    float feedbackMultiplier =  (1. - randRounded) * (1. + 0.5 * ubovert.spatialDelay_feedback);

    // Peripheral multiplier
    float peripheralProximity = distance(vec3(0, 0, 0), 0.5f * ubovert.scale) - distance(vec3(0, 0, 0), in_Position);
    float peripheralMultiplier = 1. + ubovert.spatialDelay_peripheralScale * 0.04;

    // Random noise displacement
	// float modulationAmplitude = pow(ubovert.spatialDelay_noiseDepth/20, 0.3) * 3;
	float modulationAmplitude = ubovert.spatialDelay_noiseDepth * ubovert.spatialDelay_noiseDepth * 2.;
	// in_UV1.y * 5 + in_UV1.z * 2)) * max(0, 1.0 - distance(vec3(in_UV1.x, in_UV1.y, 0), vec3(0.5, 0.5, 0)))
	float modulationNoise = 0.5 + 1.5 * noise(vec2(in_UV1.x * 5 + in_UV1.z * 2 + ubovert.spatialDelay_modulationTimePassed / 4., in_UV1.y * 5 + in_UV1.z * 2)) * max(0, 1.0 - distance(vec3(in_UV1.x, in_UV1.y, 0), vec3(0.5, 0.5, 0)));
	float modulationMultiplier = modulationNoise * modulationAmplitude;
	// vec3 modulationDisplacement = in * modulationAmplitude * modulationNoise;

    // float modulationAmplitude = pow(ubovert.spatialDelay_noiseDepth/100, 0.3);
    // float modulationNoise = noise(vec2(in_UV1.x + ubovert.spatialDelay_modulationTimePassed, in_UV1.y));
    // vec3 modulationDisplacement = in_Normal * modulationAmplitude * modulationNoise * max(0, 0.6 - distance(vec3(in_UV1.x, in_UV1.y, 0), vec3(0.5, 0.5, 0)));




    // vec3 feedbackPosition = in_Position * (1. - feedbackMultiplier) * (1. + 0.5 * ubovert.spatialDelay_feedback);

    // Modulate the input position for the peripheral effect
    // vec3 peripheralPosition = mix(feedbackPosition, vec3(0, 0, 0), peripheralProximity * peripheralScale * spatialDelayAmount / ubovert.scale);

    // Calculate final point position
    // vec3 position = peripheralPosition + ubovert.scale * spatialDelayAmount * (randomDisplacement + modulationDisplacement);
	vec3 position = in_Position * feedbackMultiplier * peripheralMultiplier + in_Position * modulationMultiplier;  

    // Calculate the point plane size
    float aspectRatio = ubovert.renderTargetSize.y / ubovert.renderTargetSize.x;
    vec3 relativePosition = vec3(in_RelativePosition.x * aspectRatio, in_RelativePosition.y, 0);

    // vec3 yAxis = normalize(vec3(position.x, position.y, 0));
    // vec3 xAxis = vec3(yAxis.y, -yAxis.x, 0);
    // vec3 relativePosition = in_RelativePosition.x * xAxis + in_RelativePosition.y * yAxis;


    // // Calculate the direction
    // vec3 up = vec3(0, 1, 0);
    // vec3 left = cross(pass_CameraPosition - pass_Position, up);
    // // vec3 left = cross(normalize(pass_Position - pass_CameraPosition), up);
    // vec2 direction = vec2(dot(pass_Position, left), dot(pass_Position, up));

    // pass_Direction = direction;

    // relativePosition = vec3(1,0,0);
    // relativePosition *= vec3(abs(in_Position.z), 1., abs(in_Position.x));
    // mat3 rot = calcLookAtMatrix(vec3(0,0,0), pass_Position, 0.);
    // relativePosition = relativePosition * rot;
    // relativePosition *= distance(ubovert.cameraPosition, vec3(0,0,0)) / 20.;

//
//    // Adjust point size for overal sound object scale, with a maximum of 10
//    float pointScaleMultiplier = max(10, (ubovert.scale.x + ubovert.scale.y + ubovert.scale.z)/3.0) * 0.012;

	// create new model matrix that only takes into account the translation.


    mat4 matrix;
    matrix[0] = vec4(1.0, 0.0, 0.0, 0.0); // first column
    matrix[1] = vec4(0.0, 1.0, 0.0, 0.0); // second column
    matrix[2] = vec4(0.0, 0.0, 1.0, 0.0); // third column
    matrix[3] = vec4(mvp.modelMatrix[3][0], mvp.modelMatrix[3][1], mvp.modelMatrix[3][2], 1.0); // fourth column



    float scaleAverage = (ubovert.scale.x + ubovert.scale.y + ubovert.scale.y) / 3.;
    scaleAverage *= 0.66;
    // float boundScale = max(1., scaleAverage);
    float boundScale = 1. + scaleAverage * 0.2;

	// keep minimum of scale
	// vec3 boundScale = vec3(max(1,ubovert.scale.x), max(1,ubovert.scale.y), max(1,ubovert.scale.z));

	// correctly positioning the SphereMesh around the sound object transform box
	vec3 postScalePosition = boundScale * position;

    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * matrix * vec4(postScalePosition, 1) + vec4(relativePosition * 1. * boundScale, 0);
}
