// Contains Gradient Noise algorithm by Inigo Quilez under MIT License.
// Copyright © 2013 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.



#version 450 core

// vertex shader input  
in vec3 pass_UV0;	

// uniform buffer inputs
uniform UBO
{
    uniform vec3 color;
    uniform float level;
    uniform float time;
} ubo;

// output
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


// in-out ease quad function
float ease_in_out_quad(float x) {
    float t = x; float b = 0; float c = 1; float d = 1;
    if ((t/=d/2) < 1) return c/2*t*t + b;
    return -c/2 * ((--t)*(t-2) - 1) + b;
}


void main()
{
    // cloud noise
    float uvMp = 1.;
    vec2 uv = vec2(uvMp * pass_UV0.x, uvMp * pass_UV0.y);
    mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 );
    float cloudValue = 0.5000*noise( uv + ubo.time * 2. ); uv = m*uv;
    cloudValue += 0.2500*noise( uv + ubo.time * 2.); uv = m*uv;
    cloudValue += 0.1250*noise( uv + ubo.time * 2.); uv = m*uv;
    cloudValue += 0.0625*noise( uv + ubo.time * 2. ); uv = m*uv;
    cloudValue = 0.5 + 0.5*cloudValue;

   // star fade
   float starPower = 2.f;
   float starValue = 1. - pow(sin(pass_UV0.x - 0.5), starPower);
   starValue *= 1. - 2. * pow(sin(pass_UV0.y - 0.5), starPower);

   // border fade / blob
   float borderIntensity = 1.5f + ubo.level * 2.f;
   float borderValue = 1. - abs((pass_UV0.x / 1.f) - 0.5f);
   borderValue *= 1.5 - abs((pass_UV0.y / 1.f) - 0.5f);
   borderValue = (1. - borderIntensity) * 1. + borderIntensity * borderValue;

   float outPower = 3.f;

   float outValue = pow(cloudValue * starValue * borderValue, outPower) * ubo.level;

   float whiteFeed = 0.2;

   out_Color = vec4((ubo.color.r + whiteFeed) * outValue, (ubo.color.g + whiteFeed) * outValue, (ubo.color.b + whiteFeed) * outValue, 1.0);

}

