#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, set = 0, binding = 0) uniform UBO_settings
{
	int size;
} settings_ubo;

layout (set = 0, binding = 1) uniform sampler2D hdrSampler;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutColor;

void main() 
{
    int blurRange = settings_ubo.size;
	int n = 0;
	vec2 texelSize = 1.0 / vec2(textureSize(hdrSampler, 0));
	vec3 result = vec3(0.0);
	for (int x = -blurRange; x < blurRange; x++) 
	{
		for (int y = -blurRange; y < blurRange; y++) 
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(hdrSampler, InTex + offset).rgb;
			n++;
		}
	}

	OutColor = vec4(result / (float(n)), 1.0);
}