#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InNormalW;
layout (location = 2) in vec3 InEyePosW;
layout (location = 3) in vec3 InColor;

layout (location = 0) out vec4 OutFragColor;

layout (set = 1, binding = 1) uniform sampler3D texture3d;

//! Corresponds to the C++ class Material. Stores the ambient, diffuse and specular colors for a material.
struct Material
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular; 
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

	// Note: this padding corresponds to the padding in Vulkan::Material
	vec4 pad;
};

layout (std140, set = 0, binding = 1) uniform UBO0 
{
	// Constants
	float numLights;
	vec3 garbage;

	Light lights[10];
} light_ubo;

layout (std140, set = 0, binding = 2) uniform UBO1
{
	vec3 fogColor;
	float padding;
	float fogStart;
	float fogDistance;
} per_frame_ps;

//! Computes the colors for directional light.
void ComputeDirectionalLight(Material material, int lightIndex, vec3 normal, vec3 toEye, out vec4 ambient, out vec4 diffuse, out vec4 spec)
{
	Light light = light_ubo.lights[lightIndex];

	// Initialize outputs.
	ambient = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	diffuse = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	spec    = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	// The light vector aims opposite the direction the light rays travel.
	vec3 lightVec = light.dir;

	// Add ambient term.
	ambient = material.ambient * light.material.ambient * light.intensity.x;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	if(diffuseFactor > 0.0f)
	{
		vec3 v = reflect(lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), light.material.specular.w);	// [TODO] Should use the models material reflection value instead
					
		diffuse = diffuseFactor * material.diffuse * light.material.diffuse * light.intensity.y;
		spec    = specFactor * material.specular * light.material.specular * light.intensity.z;
	}
}

//! Computes the colors for a point light.
void ComputePointLight(Material material, int lightIndex, vec3 pos, vec3 normal, vec3 toEye, out vec4 ambient, out vec4 diffuse, out vec4 spec)
{
	Light light = light_ubo.lights[lightIndex];

	// Initialize outputs.
	ambient = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = vec4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	vec3 lightVec = pos - light.pos;
		
	// The distance from surface to light.
	float d = length(lightVec);
	
	// Range test.
	if( d > light.range )
		return;
		
	// Normalize the light vector.
	lightVec /= d; 
	
	// Ambient term.
	ambient = material.ambient * light.material.ambient * light.intensity.x;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	if(diffuseFactor > 0.0f)
	{
		vec3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), light.material.specular.w);

		diffuse = diffuseFactor * material.diffuse * light.material.diffuse * light.intensity.y;
		spec    = specFactor * material.specular * light.material.specular * light.intensity.z;
	}

	// Attenuate
	float att = 1.0f / dot(light.att, vec3(1.0f, d, d*d));

	diffuse *= att;
	spec    *= att;
}

//! Computes the colors for a spot light.
void ComputeSpotLight(Material material, int lightIndex, vec3 pos, vec3 normal, vec3 toEye, out vec4 ambient, out vec4 diffuse, out vec4 spec)
{
	Light light = light_ubo.lights[lightIndex];

	// Initialize outputs.
	ambient = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = vec4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	vec3 lightVec = pos - light.pos;
		
	// The distance from surface to light.
	float d = length(lightVec);
	
	// Range test.
	if(d > light.range)
		return;
		
	// Normalize the light vector.
	lightVec /= d; 
	
	// Ambient term.
	ambient = material.ambient * light.material.ambient * light.intensity.x;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	if(diffuseFactor > 0.0f)
	{
		vec3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), light.material.specular.w);
					
		diffuse = diffuseFactor * material.diffuse * light.material.diffuse * light.intensity.y;
		spec    = specFactor * material.specular * light.material.specular * light.intensity.z;
	}
	
	// Scale by spotlight factor and attenuate.
	float spot = pow(max(dot(lightVec, light.dir), 0.0f), light.spot);

	// Scale by spotlight factor and attenuate.
	float att = spot / dot(light.att, vec3(1.0f, d, d*d));

	ambient *= spot;
	diffuse *= att;
	spec    *= att;
}

//! Takes a list of lights and calculate the resulting color for the pixel after all light calculations.
void ApplyLighting(Material material, vec3 posW, vec3 normalW, vec3 toEyeW, vec4 texColor,
				   float shadow, out vec4 litColor)
{
	// Start with a sum of zero. 
	vec4 ambient = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	vec4 diffuse = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	vec4 spec    = vec4(0.0f, 0.0f, 0.0f, 0.0f);

	// Loop through all lights
	for(int i = 0; i < light_ubo.numLights; i++)
	{
		// Sum the light contribution from each light source.
		vec4 A, D, S;

		if(light_ubo.lights[i].type == 0.0f)			// Directional light
			ComputeDirectionalLight(material, i, normalW, toEyeW, A, D, S);
		else if(light_ubo.lights[i].type == 1.0f)		// Point light
			ComputePointLight(material, i, posW, normalW, toEyeW, A, D, S);
		else if(light_ubo.lights[i].type == 2.0f)		// Spot light
			ComputeSpotLight(material, i, posW, normalW, toEyeW, A, D, S);

		ambient += A;  
		diffuse += shadow*D;
		spec    += shadow*S;
	}
	   
	litColor = texColor*(ambient + diffuse) + spec;
}


void main(void)
{
	// Calculate the color based on height
	vec3 biomeColor = vec3(1.0f, 1.0f, 1.0f);
	
	biomeColor = mix(biomeColor, vec3(0, 1, 0), InPosW.y / 5000.0);
	biomeColor = min(biomeColor, vec3(1, 1, 1));

	// Interpolating normal can unnormalize it, so normalize it.
    vec3 normalW = normalize(InNormalW); 

	vec3 toEyeW = normalize(InEyePosW - InPosW);

	float shadow = 1.0f;
	vec4 texColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	Material material;
	material.ambient = vec4(biomeColor, 1.0f); 
	material.diffuse = vec4(biomeColor, 1.0f); 
	material.specular = vec4(biomeColor, 1.0f); 

	vec4 litColor;
	ApplyLighting(material, InPosW, normalW, toEyeW, texColor, shadow, litColor);

	// Apply fogging.
	float distToEye = length(InEyePosW + InPosW); // TODO: NOTE: This should be "-". Related to the negation of the world matrix push constant.
	float fogLerp = clamp((distToEye - per_frame_ps.fogStart) / per_frame_ps.fogDistance, 0.0, 1.0); 

	// Blend the fog color and the lit color.
	litColor = vec4(mix(litColor.rgb, per_frame_ps.fogColor, fogLerp), 1.0f);

	//if (per_frame_ps.fogStart == 41500.0f)
	//	litColor = vec4(1, 0, 0, 1);

	OutFragColor = litColor;
}