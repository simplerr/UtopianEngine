#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosL;			// Vertex in local coordinate system
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;		// Normal in local coordinate system
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec3 InTangentL;
layout (location = 5) in vec3 InBitangentL;

layout (location = 0) out vec3 OutPosW;
layout (location = 1) out vec3 OutNormalW;
layout (location = 2) out vec3 OutEyePosW;
layout (location = 3) out vec3 OutColor;
layout (location = 4) out vec2 OutTex;

layout (std140, set = 0, binding = 0) uniform UBO 
{
	// Camera 
	mat4 projection;
	mat4 view;
	
	vec4 clippingPlane;
	vec3 eyePos;

	float t;
	
	// Constans
	bool useInstancing;
	vec3 garbage;
} per_frame_vs;

layout(push_constant) uniform PushConsts {
	 mat4 world;		
	 mat4 worldInvTranspose;
	 vec4 color;	
} pushConsts;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_ClipDistance[];
};

void main() 
{
	// Note: workaround to avoid glslang to optimize unused inputs
	vec3 temp = InTangentL;
	temp = InBitangentL;

	OutColor = pushConsts.color.rgb; 
	OutEyePosW = per_frame_vs.eyePos;

	// Transform to world space.
	OutPosW     = (pushConsts.world * vec4(InPosL, 1.0f)).xyz;
	OutNormalW  = mat3(pushConsts.worldInvTranspose) * InNormalL;
	//OutTangentW = mul(InTangentL, pushConsts.world);

	// Pass on the texture coordinates.
	OutTex = InTex;

	// Transform to homogeneous clip space.
	gl_Position = per_frame_vs.projection * per_frame_vs.view * pushConsts.world * vec4(InPosL.xyz, 1.0);

	gl_ClipDistance[0] = dot(pushConsts.world * vec4(InPosL.xyz, 1.0), per_frame_vs.clippingPlane);	
}