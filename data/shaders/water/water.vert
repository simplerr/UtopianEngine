#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosL;			// Vertex in local coordinate system
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;		// Normal in local coordinate system
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec4 InTangentL;

layout (location = 0) out vec4 OutClipSpace;

layout (std140, set = 0, binding = 0) uniform UBO 
{
	// Camera 
	mat4 projection;
	mat4 view;
	vec3 eyePos;
} per_frame_vs;

layout(push_constant) uniform PushConsts {
	 mat4 world;		
	 vec3 color;
} pushConsts;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main(void)
{
	OutClipSpace = per_frame_vs.projection * per_frame_vs.view * pushConsts.world * vec4(InPosL.xyz, 1.0);
	gl_Position = OutClipSpace;
}