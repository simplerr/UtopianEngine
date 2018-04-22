#version 450

layout (location = 0) in vec3 InColor;
layout (location = 1) in vec3 InPosW;
layout (location = 2) in vec3 InNormalW;
layout (location = 3) in vec2 InTex;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

layout (set = 1, binding = 0) uniform sampler2D texSampler;

void main() 
{
	vec4 texColor = texture(texSampler, InTex);

	outPosition = vec4(InPosW, 1.0f);
	outNormal = vec4(normalize(InNormalW), 1.0f);
	outAlbedo = vec4(texColor);
}