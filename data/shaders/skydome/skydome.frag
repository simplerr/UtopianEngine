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
	vec3 eyePos;

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

/*
 * From https://www.shadertoy.com/view/4slGD4
 */

#define HASHSCALE1 .1031
#define HASHSCALE3 vec3(.1031, .1030, .0973)
#define HASHSCALE4 vec4(1031, .1030, .0973, .1099)
#define CLOUD_QUALITY 7
#define CLOUD_SPEED 5000.0f

vec2 add = vec2(1.0, 0.0);

float Hash12(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}
vec2 Hash22(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * HASHSCALE3);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.xx + p3.yz) * p3.zy);
}

float Noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    
    float res = mix(mix(Hash12(p), Hash12(p + add.xy), f.x),
                    mix(Hash12(p + add.yx), Hash12(p + add.xx),f.x), f.y);
    return res;
}
 
float FractalNoise(in vec2 xy)
{
	float w = .7;
	float f = 0.0;

	for (int i = 0; i < CLOUD_QUALITY; i++)
	{
		f += Noise(xy) * w;
		w *= 0.5;
		xy *= 2.7;
	}

	return f;
}

vec3 GetClouds(in vec3 skyColor, in vec3 cloudColor, in vec3 rd)
{
	//if (rd.y < 0.01) return sky;
	vec3 cameraPos = ubo_parameters.eyePos;

	float v = (200.0 - cameraPos.y) / rd.y;
	rd.xz *= v;
	rd.xz += cameraPos.xz;
	rd.xz *= .010;
	float f = (FractalNoise(rd.xz + ubo_parameters.time / CLOUD_SPEED) - 0.55) * 5.0;

	// Uses the ray's y component for horizon fade of fixed colour clouds...
	vec3 finalColor = mix(skyColor, cloudColor, clamp(f * rd.y - 0.1, 0.0, 1.0));

	return finalColor;
}

void main() 
{
	float radius = 1.0f;
	float inclination = ubo_parameters.inclination;
	float azimuth = ubo_parameters.azimuth;// * 0.00001;

	vec3 unitPos = normalize(InPosL);

	vec3 skyColor = vec3(0.0);

	// Sky
	if (ubo_parameters.onlySun == 0)
		skyColor = mix(horizonColor, zenithColor, pow(abs(unitPos.y), 0.7));

	// Sun
	vec3 sunPos = vec3(radius * sin(inclination) * cos(azimuth),
					   radius * cos(inclination),
					   radius * sin(inclination) * sin(azimuth));

	float sun = 10 * pow(max(dot(sunPos, unitPos), 0.0), 500.0);

	// Todo: Remove duplicate calls to GetClouds()
	float sunCloudMask = GetClouds(vec3(0.0f), vec3(1.0f), unitPos).x;
	sun *= (1.0f - sunCloudMask);

	vec3 cloudColor = vec3(0.85f);
	OutFragColor = vec4(GetClouds(skyColor, cloudColor, unitPos), 1.0f);
	OutSun = vec4(sun * sunColor, 1.0);
}