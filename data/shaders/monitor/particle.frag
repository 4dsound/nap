#version 330 core

// vertex shader input
in vec3 passNormal;                        //< frag normal in world space
in vec3 passPosition;                    //< frag world space position
in vec4 passColor;                        //< frag color

uniform UBO {
    // uniform inputs
    uniform vec3 inCameraPosition;            //< Camera World Space Position
    uniform float intensity;
} ubo;

// output
out vec4 out_Color;

void main()
{
    vec3 halo_color = vec3(0.0, 0.0, 0.0);
    vec3 intensity_color = vec3(1.0, 1.0, 1.0);
    
    vec3 base_color = mix(passColor.rgb, intensity_color, ubo.intensity);
    
    // Calculate mesh to camera angle for halo effect
    vec3 cam_normal = normalize(ubo.inCameraPosition - passPosition);
    
    float cam_surface_dot = clamp(dot(normalize(passNormal), cam_normal), 0.0, 1.0);
    cam_surface_dot = clamp((1.0 - cam_surface_dot) - 0.1, 0.0, 1.0);
    cam_surface_dot = pow(cam_surface_dot, 2.0);
    
    // Mix in the halo, set fragment color output
    out_Color.rgb = mix(base_color, halo_color, cam_surface_dot);
    out_Color.a = passColor.a;
}