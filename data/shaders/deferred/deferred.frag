#version 450

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

layout (set = 1, binding = 0) uniform sampler2D positionSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;
layout (set = 1, binding = 2) uniform sampler2D albedoSampler;

layout (std140, set = 0, binding = 0) uniform UBO 
{
	vec4 EyePosW;
} eye_ubo;


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

layout (std140, set = 0, binding = 1) uniform UBO1 
{
	// Constants
	float numLights;
	vec3 garbage;

	Light lights[10];
} light_ubo;

layout (std140, set = 0, binding = 2) uniform UBO2
{
	vec3 fogColor;
	float padding;
	float fogStart;
	float fogDistance;
} fog_ubo;

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
	vec3 lightVec = light.pos - pos;
		
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

	//diffuse *= att;
	//spec    *= att;
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
	vec3 lightVec = light.pos - pos;
		
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

void main() 
{
	vec2 uv = InTex;
	uv.x *= -1;

	vec3 position = texture(positionSampler, uv).rgb;
	vec3 normal = texture(normalSampler, uv).rgb;
	vec3 albedo = texture(albedoSampler, uv).rgb;

	vec3 toEyeW = normalize(eye_ubo.EyePosW.xyz - position);

	float shadow = 1.0f;

	Material material;
	material.ambient = vec4(1.0f, 1.0f, 1.0f, 1.0f); 
	material.diffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f); 
	material.specular = vec4(1.0f, 1.0f, 1.0f, 1.0f); 

	vec4 litColor;
	ApplyLighting(material, position, normal, toEyeW, vec4(albedo, 1.0f), shadow, litColor);

	// Apply fogging.
	float distToEye = length(eye_ubo.EyePosW.xyz + position); // TODO: NOTE: This should be "-". Related to the negation of the world matrix push constant.
	float fogLerp = clamp((distToEye - fog_ubo.fogStart) / fog_ubo.fogDistance, 0.0, 1.0); 

	// Blend the fog color and the lit color.
	litColor = vec4(mix(litColor.rgb, fog_ubo.fogColor, fogLerp), 1.0f);

	OutFragColor = litColor;
	// float depth = texture(positionSampler, uv).w;// / 100000.0f;
	// OutFragColor = vec4(depth, depth, depth, 1.0f);
}
