#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

layout (set = 0, binding = 0) uniform sampler2D reflectionSampler;
layout (set = 0, binding = 1) uniform sampler2D refractionSampler;
layout (set = 0, binding = 3) uniform sampler2D distortionSampler;
layout (set = 0, binding = 4) uniform sampler2D positionSampler;
layout (set = 0, binding = 5) uniform sampler2D normalSampler;
layout (set = 0, binding = 6) uniform sampler2D albedoSampler;

layout (set = 0, binding = 7) uniform UBO_parameters
{
    vec4 ubo_parameters;
} ubo_parameters;

void main() 
{
    float reflectivity = texture(albedoSampler, InTex).a;

    if (reflectivity == 0.0f)
    {
        OutFragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        return;
    }

    vec3 position = texture(positionSampler, InTex).xyz;
    vec3 normal = texture(normalSampler, InTex).xyz;

    // Todo: Note: the + sign is due to the fragment world position is negated for some reason
	// this is a left over from an old problem
	vec3 toEyeW = normalize(ubo_parameters.ubo_parameters.xyz + position);
    float refractivity = dot(toEyeW, normal);

    vec2 distortion = texture(distortionSampler, InTex).rg;
    vec3 reflectionColor = texture(reflectionSampler, InTex + distortion).rgb;
    vec3 refractionColor = texture(refractionSampler, InTex + distortion).rgb;

    vec3 finalColor = mix(reflectionColor, refractionColor, refractivity);//0.2f);

    //OutFragColor = vec4(reflectionColor / 2.0f, 1.0f);
    OutFragColor = vec4(finalColor / 2.0f, 1.0f);
    //OutFragColor = vec4(distortion, 0, 1);
}