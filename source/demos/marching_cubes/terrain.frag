#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InNormalW;
layout (location = 2) in vec3 InEyePosW;
layout (location = 3) in vec3 InColor;

layout (location = 0) out vec4 OutFragColor;

void main(void)
{
    vec3 tmp = InPosW;
    tmp = InEyePosW;
    tmp = InColor;

    OutFragColor = vec4(InNormalW, 1.0f);
    //OutFragColor *= vec4(InColor, 1.0f);
}