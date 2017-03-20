#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 InPosL;
layout (location = 1) in vec4 InNormal;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;

layout (std140, set = 0, binding = 0) uniform UBO 
{
	// Camera 
	mat4 projection;
	mat4 view;
} per_frame;

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
	outColor = vec3(0, 1, 0);
	outNormal = vec3(1, 1, 1);

	gl_Position = per_frame.projection * per_frame.view * pushConsts.world * vec4(InPosL.xyz, 1.0);
	//gl_Position = vec4(InPosL.xyz, 1.0);
}