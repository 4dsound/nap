#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform float value = 1.0;
uniform float scaleX = 1.0;

in vec3	in_Position;
in vec4	in_Color0;

out vec4 pass_Color;

void main(void)
{
    vec3 right = vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
    vec3 up = vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);
    vec3 offset = vec3(0.0, 0.5, 0.0);
    vec3 position = in_Position;
    
    // spherical billboard
    position = (right * (position.x + offset.x) * scaleX) + (up * (position.y + offset.y) * value);
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);

    pass_Color = in_Color0;
}