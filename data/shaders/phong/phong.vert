#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosL;			// Vertex in local coordinate system
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;		// Normal in local coordinate system
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec4 InTangentL;

layout (std140, set = 0, binding = 0) uniform UBO 
{
	// Camera 
	mat4 projection;
	mat4 view;
	
	vec4 lightDir;
	vec3 eyePos;

	float t;
	
	// Constans
	bool useInstancing;
	vec3 garbage;
} per_frame_vs;

layout(push_constant) uniform PushConsts {
	 mat4 world;		
	 mat4 worldInvTranspose;
	 //vec3 color;	
} pushConsts;

layout (location = 0) out vec3 OutPosW;
layout (location = 1) out vec3 OutNormalW;
layout (location = 2) out vec3 OutEyePosW;
layout (location = 3) out vec3 OutColor;
layout (location = 4) out vec2 OutTex;

void main() 
{
	OutColor = vec3(1.0f, 1.0f, 1.0f);	//pushConsts.color; 
	OutEyePosW = per_frame.eyePos;

	// Transform to world space.
	OutPosW     = (pushConsts.world * vec4(InPosL, 1.0f)).xyz;
	OutNormalW  = mat3(pushConsts.worldInvTranspose) * InNormalL;
	//OutTangentW = mul(InTangentL, pushConsts.world);

	// Pass on the texture coordinates.
	OutTex = InTex;

	// Transform to homogeneous clip space.
	gl_Position = per_frame.projection * per_frame.view * pushConsts.world * vec4(InPosL.xyz, 1.0);
}