#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InPosL;
layout (location = 2) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;
layout (location = 1) out vec4 OutSun;

layout (set = 0, binding = 1) uniform UBO_parameters 
{
	float sphereRadius;
	float inclination;
	float azimuth;
	float time;
	float sunSpeed;

	// Used by the sun shaft job to create the radial blur effect
	int onlySun;
} ubo_parameters;

// Dusk
// const vec3 zenithColor = vec3(0.5f, 0.4f, 0.3f);
// const vec3 horizonColor = vec3(0.9f, 0.4f, 0.1f);

// Noon
const vec3 zenithColor = vec3(0.1f, 0.4f, 1.f);
const vec3 horizonColor = vec3(0.34f, 0.54f, 0.88f);

const vec3 sunColor = vec3(.6f, .35f, 0.2f);
const float sunSize = 0.10;

void main() 
{
	float radius = 1.0f;
	float inclination = ubo_parameters.inclination;
	float azimuth = ubo_parameters.azimuth;// * 0.00001;

	vec3 unitPos = normalize(InPosL);

	vec3 color = vec3(0.0);

	// Sky
	if (ubo_parameters.onlySun == 0)
		color = mix(horizonColor, zenithColor, pow(1*abs(unitPos.y), 0.7));

	// Sun
	vec3 sunPos = vec3(radius * sin(inclination) * cos(azimuth),
					   radius * cos(inclination),
					   radius * sin(inclination) * sin(azimuth));

	float sun = 50 * pow(max(dot(sunPos, unitPos), 0.0), 500.0);
	color += sun * sunColor;

	OutFragColor = vec4(color, 1.0);
	OutSun = vec4(sun * sunColor, 1.0);
}