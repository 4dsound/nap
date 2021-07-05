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
} ubovert;


in vec3 in_Position;
in vec3 in_RelativePosition;
in vec3	in_UV0;

out vec3 pass_UV0;

void main(void)
{
    pass_UV0 = in_UV0;
    float distanceToCamera = distance(ubovert.cameraPosition, in_Position);
    float aspectRatio = ubovert.renderTargetSize.y / ubovert.renderTargetSize.x;
    vec3 size = vec3(in_RelativePosition.x * aspectRatio, in_RelativePosition.y, 0) * 0.05;
    gl_Position = (mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0)) + vec4(size, 0);
}
