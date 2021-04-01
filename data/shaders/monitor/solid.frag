#version 330 core

// vertex shader input
in vec3 passNormal;                        //< frag normal in world space
in vec3 passPosition;                    //< frag world space position

uniform UBO {
    // uniform inputs
    uniform float intensity;
    uniform vec3 color;
    uniform vec3 intensityColor;
} ubo;

// output
out vec4 out_Color;

void main()
{
    
    vec3 outColor = mix(ubo.color, ubo.intensityColor, ubo.intensity);
    
    out_Color.rgb = outColor;
    out_Color.a = 0;
}
