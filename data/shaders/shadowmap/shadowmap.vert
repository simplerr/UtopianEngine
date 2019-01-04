#version 450

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec4 InTangentL;

layout (std140, set = 0, binding = 0) uniform UBO_cascadeTransforms 
{
	mat4 viewProjection[4];
} cascade_transforms;

layout (push_constant) uniform PushConstants {
	 mat4 world;
	 uint cascadeIndex;
} pushConstants;

layout (location = 0) out vec2 OutTex;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main() 
{
	OutTex = InTex;

	// Note: workaround to avoid glslang to optimize unused inputs
	vec3 temp = InColor;
	temp = InNormalL;
	vec4 temp2 = InTangentL;

	gl_Position = cascade_transforms.viewProjection[pushConstants.cascadeIndex] * pushConstants.world * vec4(InPosL.xyz, 1.0);
}