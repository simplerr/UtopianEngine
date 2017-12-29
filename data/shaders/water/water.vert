#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosL;			// Vertex in local coordinate system
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;		// Normal in local coordinate system
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec4 InTangentL;

layout (location = 0) out vec2 OutTexCoord;
layout (location = 1) out vec4 OutClipSpace;
layout (location = 2) out float OutMoveFactor;
layout (location = 3) out vec3 OutToEye;

layout (std140, set = 0, binding = 0) uniform UBO 
{
	// Camera 
	mat4 projection;
	mat4 view;
	vec3 eyePos;
	float moveFactor;
} per_frame_vs;

layout(push_constant) uniform PushConsts {
	 mat4 world;		
	 vec3 color;
} pushConsts;

out gl_PerVertex
{
	vec4 gl_Position;
};

const float tiling = 0.00025;

void main(void)
{
	vec4 worldPos = pushConsts.world * vec4(InPosL.xyz, 1.0);
	OutToEye = per_frame_vs.eyePos - worldPos.xyz;
	//OutToEye =  vec3(67200, 2700, 67200) - worldPos.xyz;
	OutClipSpace = per_frame_vs.projection * per_frame_vs.view * pushConsts.world * vec4(InPosL.xyz, 1.0);
	gl_Position = OutClipSpace;
	OutTexCoord = vec2(InPosL.x / 2.0f + 0.5f, InPosL.z / 2.0f + 0.5f) * tiling;
	OutMoveFactor = per_frame_vs.moveFactor;
}