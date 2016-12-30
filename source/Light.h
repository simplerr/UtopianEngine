#pragma once
#include <glm/glm.hpp>

using namespace glm;

namespace VulkanLib
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

		/*Material(aiColor4D ambient, aiColor4D diffuse, aiColor4D specular) {
		this->ambient.x = ambient.r;
		this->ambient.y = ambient.g;
		this->ambient.z = ambient.b;
		this->ambient.w = ambient.a;

		this->diffuse.x = diffuse.r;
		this->diffuse.y = diffuse.g;
		this->diffuse.z = diffuse.b;
		this->diffuse.w = diffuse.a;

		this->specular.x = specular.r;
		this->specular.y = specular.g;
		this->specular.z = specular.b;
		this->specular.w = specular.a;
		}*/

		vec4 ambient;
		vec4 diffuse;
		vec4 specular; // w = SpecPower
	};


	class Light
	{
	public:
		Light();

		// Setters
		void SetMaterials(vec4 ambient, vec4 diffuse, vec4 specular);
		void SetPosition(vec3 position);
		void SetPosition(float x, float y, float z);
		void SetDirection(vec3 direction);
		void SetDirection(float x, float y, float z);
		void SetRange(float range);
		void SetSpot(float spot);
		void SetAtt(float a0, float a1, float a2);
		void SetType(LightType type);
		void SetIntensity(float ambient, float diffuse, float specular);
		void SetId(int id);

		// Getters
		vec3 GetPosition();
		vec3 GetDirection();
		vec3 GetAtt();
		vec3 GetIntensity();
		Material GetMaterial();
		float	 GetRange();
		float	 GetSpot();
		int		 GetId();
		int		 GetType();

	private:
		// Light color
		Material mMaterial;

		// Packed into 4D vector: (position, range)
		vec3	mPosition;
		float	mRange;

		// Packed into 4D vector: (direction, spot)
		vec3	mDirection;
		float	mSpot;

		// Packed into 4D vector: (att, type)
		vec3	mAtt;
		float	mType;	// 0 = directional, 1 = point light, 2 = spot light

						// Light intensity
		vec3	mIntensity;
		float	mId;
	};
}