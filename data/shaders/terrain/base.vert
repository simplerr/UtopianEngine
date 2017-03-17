#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPos;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;

layout(push_constant) uniform PushConsts {
	 mat4 world;		
	 mat4 worldInvTranspose;
} pushConsts;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main(void)
{
	outColor = vec3(1, 1, 1);
	outNormal = vec3(1, 1, 1);

	gl_Position = pushConsts.world * vec4(InPos.xyz, 1.0);
}