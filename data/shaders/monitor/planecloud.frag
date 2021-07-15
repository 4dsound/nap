#version 450 core

uniform UBO
{
    uniform vec3 color;
} ubo;

in vec3 pass_UV0;
in vec3 pass_Position;
in vec3 pass_CameraPosition;
in vec3 pass_Scale;
out vec4 out_Color;

void main()
{
    float distanceToCenter = distance(pass_UV0, vec3(0.5, 0.5, 0));
    float distanceToCamera = distance(pass_Position, pass_CameraPosition);
    float scaleFactor = (pass_Scale.x + pass_Scale.y + pass_Scale.z)/3.0;
    float alpha = max(0.2, (distanceToCamera * 0.5) / max(20, scaleFactor));
    if (distanceToCenter > 0.5)
        alpha = 0;
    out_Color = vec4(ubo.color, alpha);
}
