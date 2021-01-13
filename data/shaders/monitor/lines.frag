#version 450 core
uniform UBO
{
    uniform vec3 color;
    uniform vec3 cameraPosition;
} ubo;

in vec3 pass_Position;
out vec4 out_Color;
void main()
{
    float cameraDistance = length(ubo.cameraPosition - pass_Position);
    float maxCameraDistance = 7;
    float distanceFade =  1 - 0.5 * (cameraDistance / maxCameraDistance);
    vec3 fadedColor = mix(vec3(0.1, 0.1, 0.1), ubo.color, distanceFade);
    out_Color = vec4(fadedColor, 1.0);
}