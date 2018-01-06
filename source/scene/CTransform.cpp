#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "scene/CTransform.h"

namespace Scene
{
	CTransform::CTransform(SceneEntity* parent, vec3 position)
		: SceneComponent(parent)
	{
		SetName("CTransform");
		SetPosition(position);
		SetRotation(vec3(0, 0, 0));
		SetScale(vec3(1.0f, 1.0f, 1.0f));
	}

	CTransform::~CTransform()
	{
	}

	void CTransform::SetPosition(vec3 position)
	{
		mPosition = position;
		RebuildWorldMatrix();
	}

	void CTransform::SetRotation(vec3 rotation)
	{
		mRotation = rotation;
		RebuildWorldMatrix();
	}

	void CTransform::SetScale(vec3 scale)
	{
		mScale = scale;
		RebuildWorldMatrix();
	}

	void CTransform::AddRotation(float x, float y, float z)
	{
		mRotation += vec3(x, y, z);
		RebuildWorldMatrix();
	}

	void CTransform::AddScale(float x, float y, float z)
	{
		mScale += vec3(x, y, z);
		RebuildWorldMatrix();
	}

	void CTransform::AddScale(vec3 scale)
	{
		mScale += scale;
		RebuildWorldMatrix();
	}

	vec3 CTransform::GetPosition()
	{
		return mPosition;
	}

	vec3 CTransform::GetRotation()
	{
		return mRotation;
	}

	vec3 CTransform::GetScale()
	{
		return mScale;
	}

	mat4 CTransform::GetWorldMatrix()
	{
		return mWorld;
	}

	mat4 CTransform::GetWorldInverseTransposeMatrix()
	{
		return glm::inverseTranspose(mWorld);
	}

	void CTransform::RebuildWorldMatrix()
	{
		mat4 world;

		world = glm::translate(world, mPosition);
		world = glm::rotate(world, glm::radians(mRotation.x), vec3(1.0f, 0.0f, 0.0f));
		world = glm::rotate(world, glm::radians(mRotation.y), vec3(0.0f, 1.0f, 0.0f));
		world = glm::rotate(world, glm::radians(mRotation.z), vec3(0.0f, 0.0f, 1.0f));
		world = glm::scale(world, mScale);

		mWorld = world;
	}
}