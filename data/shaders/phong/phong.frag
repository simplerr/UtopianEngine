#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D samplerColorMap;

layout (location = 0) in vec3 InNormalW;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec2 InTex;
layout (location = 3) in vec3 InEyeDirW;

//! Corresponds to the C++ class Material. Stores the ambient, diffuse and specular colors for a material.
struct Material
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular; // w = SpecPower
};

struct Light
{
	// Color
	Material material;

	vec3 pos;
	float range;

	vec3 dir;
	float spot;

	vec3 att;
	float type;

	vec3 intensity;
	float id;
};

layout (std140, binding = 1) uniform UBO 
{
	Light light[1];

	// Constants
	float numLights;
	vec3 garbage;
} per_frame;

layout (location = 0) out vec4 OutFragColor;

void main() 
{
	// Ambient factor
	vec3 color = 0.2 * InColor;	// Ambient
	vec3 Color = per_frame.light[0].material.ambient.rgb;	

	vec3 normal = normalize(InNormalW);
	vec3 lightDir = normalize(per_frame.light[0].dir);
	vec3 V = normalize(InEyeDirW);
	vec3 R = reflect(-lightDir, normal);

	// Diffuse
	float shade = clamp(dot(normal, lightDir), 0.0f, 1.0f);
	vec3 diffuse = shade * Color;
	color += diffuse;

	// Specular
	shade = pow(max(dot(R, V), 0.0), 512.0);
	vec3 specular = shade * Color;
	color += specular;	

	OutFragColor = vec4(color, 1.0f);

	//OutFragColor = vec4(InColor, 1.0f);
}