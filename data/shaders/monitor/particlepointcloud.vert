#version 150 core

// NAP uniform buffer objects, contains all the matrices
uniform nap
{
    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 modelMatrix;
} mvp;

uniform UBOVert
{
    uniform vec3 scale;
    uniform vec3 cameraPosition;
} ubovert;

in vec3	in_Position;
in int in_Index;

out float pass_show;
out float pass_alpha;
out float pass_cameraDistance;

void main(void)

{
    float size = ubovert.scale.x * ubovert.scale.y * ubovert.scale.z;
    float cameraDistance = length(in_Position - ubovert.cameraPosition);
    int pointCount = int(pow(size, 0.6) * 1000000);
    if (length(in_Position - vec3(0, 0, 0)) > 0.5)
        pass_show = 0;
    else
        pass_show = 1;

    if (in_Index > pointCount)
        pass_show = 0;

    pass_alpha = 1 - length(in_Position - vec3(0, 0, 0));
    pass_cameraDistance = cameraDistance;
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
}