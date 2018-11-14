#version 450

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec4 InTangentL;

layout (std140, set = 0, binding = 0) uniform UBO_viewProjection 
{
	// Camera 
	mat4 projection;
	mat4 view;
} per_frame_vs;

layout (push_constant) uniform PushConstants {
	 mat4 world;
	 mat4 worldInv;
	 vec4 color;
} pushConstants;

layout (location = 0) out vec3 OutColor;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main() 
{
	OutColor = pushConstants.color.rgb;
	vec3 temp = InColor;
	temp = InNormalL;
	vec2 temp1 = InTex;
	vec4 temp2 = InTangentL;

	gl_Position = per_frame_vs.projection * per_frame_vs.view * pushConstants.world * vec4(InPosL.xyz, 1.0);
}