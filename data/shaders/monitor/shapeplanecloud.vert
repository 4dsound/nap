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
    uniform float spatialDelay_peripheralScale;
    uniform float spatialDelay_random;
    uniform float spatialDelay_noiseDepth;
    uniform float spatialDelay_noiseSpeed;
    uniform float spatialDelay_smooth;
    uniform float spatialDelay_modulationTimePassed;
//    uniform float spatialDelay_distanceScale;
//    uniform float spatialDelay_randomPatternScale;
} ubovert;


in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_RelativePosition;
in vec3	in_UV0;
in vec3 in_UV1;

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

    float amplitude = ubovert.spatialDelay_random * 0.1;
    float detail = 2 + ubovert.spatialDelay_noiseDepth * (1 - ubovert.spatialDelay_smooth) * 4;
    float speed = ubovert.spatialDelay_noiseSpeed;
    vec3 displacementX = in_Normal * noise(vec2(in_UV1.x * detail + ubovert.spatialDelay_modulationTimePassed, 0)) * max(0, 0.6 - distance(vec3(in_UV1.x, in_UV1.y, 0), vec3(0.5, 0.5, 0)));
    vec3 displacementY = in_Normal * noise(vec2(0, in_UV1.y * detail + ubovert.spatialDelay_modulationTimePassed)) * max(0, 0.6 - distance(vec3(in_UV1.x, in_UV1.y, 0), vec3(0.5, 0.5, 0)));

    float peripheralDistance = abs(distance(vec3(0, 0, 0), vec3(0.5, 0.5, 0.5) * ubovert.scale) - distance(vec3(0, 0, 0), in_Position));
    float peripheralScale = ubovert.spatialDelay_peripheralScale * 0.0125;
//    vec3 peripheralDisplacement = ((-in_Position - vec3(0, 0, 0))) * peripheralScale * ubovert.scale * (5 - peripheralDistance);

    vec3 peripheralPosition = mix(in_Position, vec3(0, 0, 0), peripheralDistance * peripheralScale);;
    vec3 position = peripheralPosition + amplitude * ubovert.scale * (displacementX + displacementY);
    gl_Position = (mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(position, 1.0)) + vec4(size, 0);
}
