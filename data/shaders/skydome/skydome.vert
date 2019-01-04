#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec4 InTangentL;

layout (set = 0, binding = 0) uniform UBO_viewProjection 
{
	mat4 projection;
	mat4 view;
	mat4 world;
} ubo;

layout (location = 0) out vec3 OutPosW;
layout (location = 1) out vec3 OutPosL;
layout (location = 2) out vec2 OutTex;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	// Note: workaround to avoid glslang to optimize unused inputs
	vec3 temp = InColor;
	temp = InNormalL;
	vec4 temp2 = InTangentL;

	OutPosW = (ubo.world * vec4(InPosL.xyz, 1.0)).xyz;
	OutPosL = InPosL.xyz;
	OutTex = InTex;

	gl_Position = ubo.projection * ubo.view * ubo.world * vec4(InPosL.xyz, 1.0);
}
