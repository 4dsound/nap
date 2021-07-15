// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core
uniform nap
{
    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 modelMatrix;
} mvp;

uniform UBOVert
{
    uniform float level;
    uniform vec3 scale;
} ubovert;

in vec3 in_Position;
in vec3 in_UV0;

out vec3 pass_UV0;

void main(void)
{
    // ensure minimum scale, don't show if the particle will be visible as a point particle.
    mat4 modelMatrix = mvp.modelMatrix;
    if(ubovert.scale.x < 0.1f && ubovert.scale.y < 0.1f && ubovert.scale.z < 0.1f)
    {
        modelMatrix[0][0] = 0.0;
        modelMatrix[1][1] = 0.0;
        modelMatrix[2][2] = 0.0;
    }
    else
    {
        if(ubovert.scale.x < 0.1f)
            modelMatrix[0][0] = 0.1f;
        if(ubovert.scale.y < 0.1f)
            modelMatrix[1][1] = 0.1f;
        if(ubovert.scale.z < 0.1f)
            modelMatrix[2][2] = 0.1f;
    }



    pass_UV0 = in_UV0;
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * modelMatrix * vec4(in_Position, 1.0);
}
