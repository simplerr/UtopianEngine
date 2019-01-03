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

layout (std140, set = 0, binding = 1) uniform UBO_cascadeTransforms 
{
	mat4 viewProjection[4];
} cascade_transforms;

layout (push_constant) uniform PushConstants {
	 mat4 world;
	 mat4 worldInv;
	 vec4 color;
	 vec2 textureTiling;
	 vec2 pad;
} pushConstants;

layout (location = 0) out vec3 OutColor;
layout (location = 1) out vec2 OutTex;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main() 
{
	OutColor = pushConstants.color.rgb;
	OutTex = InTex;

	// Note: workaround to avoid glslang to optimize unused inputs
	vec3 temp = InColor;
	temp = InNormalL;
	vec4 temp2 = InTangentL;

	gl_Position = per_frame_vs.projection * per_frame_vs.view * pushConstants.world * vec4(InPosL.xyz, 1.0);
	gl_Position = cascade_transforms.viewProjection[0] * pushConstants.world * vec4(InPosL.xyz, 1.0);
}