#version 450 core
uniform UBO
{
    uniform vec3 color;
//    uniform float time;
} ubo;

in vec3 pass_UV0;
in float pass_Level;
out vec4 out_Color;



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

void main()
{
    float sqrt_2 = 1.4142135624f;

    // angular ray pattern
    float angleToCenter = atan((pass_UV0.y - 0.5) / (pass_UV0.x - 0.5));
    int rayCount = 5;
    float angleValue = max(0, sin((angleToCenter) * rayCount));

    float distanceToCenter = min(1, distance(pass_UV0, vec3(0.5, 0.5, 0.)));
    float distanceValue = (1. - distanceToCenter) * (1. - distanceToCenter) * (1. - distanceToCenter);

    float extraValue = (1. - distanceToCenter) * (1. - distanceToCenter) * (1. - angleValue * 0.5) - distanceToCenter * distanceToCenter;

    // cloud noise
//    float uvMp = 1.;
//    vec2 uv = vec2(uvMp * pass_UV0.x, uvMp * pass_UV0.y);
//    mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 );
//    float cloudValue = 0.5000*noise( uv + ubo.time * 2. ); uv = m*uv;
//    cloudValue += 0.2500*noise( uv + ubo.time * 2.); uv = m*uv;
//    cloudValue += 0.1250*noise( uv + ubo.time * 2.); uv = m*uv;
//    cloudValue += 0.0625*noise( uv + ubo.time * 2. ); uv = m*uv;
//    cloudValue = 0.5 + 0.5*cloudValue;
    
    float cloudValue = 1.f;

    out_Color = vec4(ubo.color, distanceValue * distanceValue * pow(pass_Level, 0.75) * 0.9 * (1. - 0.5 * cloudValue)) + vec4(ubo.color, extraValue * pow(pass_Level, 0.75) * 0.1);
;
}
