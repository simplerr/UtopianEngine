#version 450

#extension GL_GOOGLE_include_directive : enable

#include "shared_variables.glsl"

layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec3 InTangentL;
layout (location = 5) in vec3 InBitangentL;

layout (push_constant) uniform PushConstants {
	mat4 world;
	vec4 color;
} pushConstants;

layout (location = 0) out vec3 OutPosW;
layout (location = 1) out vec2 OutTex;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main() 
{
	// Todo: Workaround since glslang reflection removes unused vertex input
	vec3 color = InColor;
	vec3 normal = InNormalL;
	vec2 uv = InTex;
	vec3 tangent = InTangentL;
	vec3 bitangent = InBitangentL;

	OutPosW = (pushConstants.world * vec4(InPosL.xyz, 1.0)).xyz;
	OutTex = InTex;

	gl_Position = sharedVariables.projectionMatrix * sharedVariables.viewMatrix * pushConstants.world * vec4(InPosL.xyz, 1.0);
}