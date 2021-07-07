#version 450 core

uniform nap
{
    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 modelMatrix;
} mvp;

uniform UBOVert
{
    uniform vec3 cameraPosition;
    uniform vec2 renderTargetSize;
    uniform float time;
} ubovert;


in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_RelativePosition;
in vec3	in_UV0;
in vec3 in_UV1;

out vec3 pass_UV0;

void main(void)
{
    pass_UV0 = in_UV0;
    float distanceToCamera = distance(ubovert.cameraPosition, in_Position);
    float aspectRatio = ubovert.renderTargetSize.y / ubovert.renderTargetSize.x;
    vec3 size = vec3(in_RelativePosition.x * aspectRatio, in_RelativePosition.y, 0) * 0.05;

    float amplitude = 0;
    float speed = 0;
    vec3 displacementX = in_Normal * sin(in_UV1.x * 50 + ubovert.time * speed) * max(0, 0.6 - distance(vec3(in_UV1.x, in_UV1.y, 0), vec3(0.5, 0.5, 0)));
    vec3 displacementY = in_Normal * sin(in_UV1.y * 50 + ubovert.time * speed) * max(0, 0.6 - distance(vec3(in_UV1.x, in_UV1.y, 0), vec3(0.5, 0.5, 0)));
    vec3 position = in_Position + amplitude * (displacementX + displacementY);
    gl_Position = (mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(position, 1.0)) + vec4(size, 0);
}
