#include "Light.h"

namespace VulkanLib
{
	Light::Light()
	{
		mPosition = vec3(10, 10, 10);
		mDirection = vec3(1, 1, 1);

		mMaterial = Material(vec4(1, 1, 1, 1));		// White
		mType = 0;
	}

	void Light::SetMaterials(vec4 ambient, vec4 diffuse, vec4 specular)
	{
		mMaterial = Material(ambient, diffuse, specular);
		SetIntensity(1.0f, 0.0f, 0.0f);
	}

	void Light::SetMaterial(vec4 color)
	{
		mMaterial = Material(color);
	}

	void Light::SetPosition(vec3 position)
	{
		mPosition = position;
	}

	void Light::SetPosition(float x, float y, float z)
	{
		SetPosition(vec3(x, y, z));
	}

	void Light::SetRange(float range)
	{
		mRange = range;
	}

	void Light::SetDirection(vec3 direction)
	{
		mDirection = glm::normalize(direction);
	}

	void Light::SetDirection(float x, float y, float z)
	{
		SetDirection(glm::normalize(vec3(x, y, z)));
	}

	void Light::SetSpot(float spot)
	{
		mSpot = spot;
	}

	void Light::SetAtt(float a0, float a1, float a2)
	{
		mAtt = vec3(a0, a1, a2);
	}

	void Light::SetType(LightType type)
	{
		mType = type;
	}

	void Light::SetIntensity(float ambient, float diffuse, float specular)
	{
		mIntensity = vec3(ambient, diffuse, specular);
	}

	void Light::SetId(int id)
	{
		mId = id;
	}

	int Light::GetId()
	{
		return mId;
	}

	vec3 Light::GetIntensity()
	{
		return mIntensity;
	}

	vec3 Light::GetPosition()
	{
		return mPosition;
	}

	vec3 Light::GetDirection()
	{
		return mDirection;
	}

	vec3 Light::GetAtt()
	{
		return mAtt;
	}

	float Light::GetSpot()
	{
		return mSpot;
	}

	float Light::GetRange()
	{
		return mRange;
	}

	Material Light::GetMaterial()
	{
		return mMaterial;
	}

	int	Light::GetType()
	{
		return mType;
	}
}