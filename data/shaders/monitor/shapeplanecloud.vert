#version 450 core

uniform nap
{
    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 modelMatrix;
} mvp;

uniform UBOVert
{
    uniform vec3 scale;
    uniform vec2 renderTargetSize;
    uniform float time;
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


in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_RelativePosition;
in vec3	in_UV0;
in vec3 in_UV1;
in int in_Index;

out vec3 pass_UV0;

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
    pass_UV0 = in_UV0;
    float aspectRatio = ubovert.renderTargetSize.y / ubovert.renderTargetSize.x;
    vec3 size = vec3(in_RelativePosition.x * aspectRatio, in_RelativePosition.y, 0) * 0.05;

    float spatialDelayAmount = ubovert.spatialDelay_dryWet * ubovert.spatialDelay_enable;

    float randomAmplitude = pow(ubovert.spatialDelay_random/30.0, 0.3) * 3;
    float randomDetail = 2;
    vec3 randomDisplacement = in_Normal * randomAmplitude * noise(vec2(in_UV1.x * randomDetail + 1, in_UV1.y * randomDetail + 1)) * max(0, 0.6 - distance(vec3(in_UV1.x, in_UV1.y, 0), vec3(0.5, 0.5, 0)));

    float modulationAmplitude = pow(ubovert.spatialDelay_noiseDepth/1000, 0.3) * 3;
    float modulationDetail = (1 - ubovert.spatialDelay_smooth) * 20;
    vec3 modulationDisplacement = in_Normal * modulationAmplitude * noise(vec2(in_UV1.x * modulationDetail + ubovert.spatialDelay_modulationTimePassed, in_UV1.y * modulationDetail)) * max(0, 0.6 - distance(vec3(in_UV1.x, in_UV1.y, 0), vec3(0.5, 0.5, 0)));

    float peripheralDistance = distance(vec3(0, 0, 0), 0.5f * ubovert.scale) - distance(vec3(0, 0, 0), in_Position);
    float peripheralScale = ubovert.spatialDelay_peripheralScale * 0.07;

    float feedbackRand = (in_Index % 100)/100.0 * ubovert.spatialDelay_feedback;
    feedbackRand = pow(feedbackRand, 2.0);
    float feedbackMultiplier = int(feedbackRand * 5) / 5.0;
    vec3 feedbackPosition = in_Position * (1. - feedbackMultiplier);

//    vec3 feedbackPosition = in_Position * (1 - ((in_Index % int(ubovert.spatialDelay_feedback * 5)) * 0.2 * ubovert.spatialDelay_feedback));
    vec3 peripheralPosition = mix(feedbackPosition, vec3(0, 0, 0), peripheralDistance * peripheralScale * spatialDelayAmount / ubovert.scale);
    vec3 position = peripheralPosition + ubovert.scale * spatialDelayAmount * (randomDisplacement + modulationDisplacement);
    gl_Position = (mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(position, 1.0)) + vec4(size, 0);
}
