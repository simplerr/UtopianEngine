#version 450

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec4 InTangentL;

// Instance data
layout (location = 5) in vec4 InInstancePosW;

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
	 vec2 textureTiling;
	 vec2 pad;
} pushConstants;

layout (location = 0) out vec3 OutColor;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main() 
{
	OutColor = vec3(1, 1, 1);

	// Note: workaround to avoid glslang to optimize unused inputs
	vec3 temp = InColor;
	temp = InNormalL;
	vec2 temp1 = InTex;
	vec4 temp2 = InTangentL;

	// Todo: correct this
	vec3 instancePos = InInstancePosW.xyz;
	instancePos.xz *= -1;

	gl_Position = per_frame_vs.projection * per_frame_vs.view * vec4(50.0f * (InPosL.xyz) + instancePos.xyz, 1.0);
}