#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec4 InTangentL;

layout (location = 0) out vec3 outNormal;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main(void)
{
	// Note: workaround to avoid glslang to optimize unused inputs
	vec3 temp = InColor;
	vec2 temp1 = InTex;
	vec4 temp2 = InTangentL;

	outNormal = InNormalL;
	gl_Position = vec4(InPosL.xyz, 1.0);
}