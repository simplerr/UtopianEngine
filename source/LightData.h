#pragma once

#include <glm/glm.hpp>

using namespace glm;

namespace Vulkan
{
	enum LightType
	{
		DIRECTIONAL_LIGHT,
		POINT_LIGHT,
		SPOT_LIGHT
	};

	struct Material
	{
		Material() {
			//ZeroMemory(this, sizeof(this));
		}

		Material(vec4 color) {
			ambient = diffuse = specular = color;
		}

		Material(vec4 ambient, vec4 diffuse, vec4 specular) {
			this->ambient = ambient;
			this->diffuse = diffuse;
			this->specular = specular;
		}

		vec4 ambient;
		vec4 diffuse;
		vec4 specular;	// w = SpecPower
	};


	/* This matches the struct in the shader. */
	struct LightData
	{
	public:
		// Light color
		Material material;

		// Packed into 4D vector: (position, range)
		vec3	position;
		float	range;

		// Packed into 4D vector: (direction, spot)
		vec3	direction;
		float	spot;

		// Packed into 4D vector: (att, type)
		vec3	att;
		float	type;	// 0 = directional, 1 = point light, 2 = spot light

		// Light intensity
		vec3	intensity;
		float	padding;
	};
}