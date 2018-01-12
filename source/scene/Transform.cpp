#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "Transform.h"

namespace Scene
{
	Transform::Transform(const vec3& position)
	{
		SetPosition(position);
		SetRotation(vec3(0, 0, 0));
		SetScale(vec3(1.0f, 1.0f, 1.0f));
	}

	Transform::Transform()
	{
		SetPosition(vec3(0.0));
		SetRotation(vec3(0.0));
		SetScale(vec3(1.0));
	}

	Transform::~Transform()
	{
	}

	void Transform::SetPosition(const vec3& position)
	{
		mPosition = position;
		RebuildWorldMatrix();
	}

	void Transform::SetRotation(const vec3& rotation)
	{
		mRotation = rotation;
		RebuildWorldMatrix();
	}

	void Transform::SetScale(const vec3& scale)
	{
		mScale = scale;
		RebuildWorldMatrix();
	}

	void Transform::AddTranslation(const vec3& translation)
	{
		mPosition += translation;
	}

	void Transform::AddRotation(float x, float y, float z)
	{
		mRotation += vec3(x, y, z);
		RebuildWorldMatrix();
	}
	
	void Transform::AddRotation(const vec3& rotation)
	{
		mRotation += rotation;
		RebuildWorldMatrix();
	}

	void Transform::AddScale(float x, float y, float z)
	{
		mScale += vec3(x, y, z);
		RebuildWorldMatrix();
	}

	void Transform::AddScale(const vec3& scale)
	{
		mScale += scale;
		RebuildWorldMatrix();
	}

	const vec3& Transform::GetPosition() const
	{
		return mPosition;
	}

	const vec3& Transform::GetRotation() const
	{
		return mRotation;
	}

	const vec3& Transform::GetScale() const
	{
		return mScale;
	}

	const mat4& Transform::GetWorldMatrix() const
	{
		return mWorld;
	}

	const mat4& Transform::GetWorldInverseTransposeMatrix() const
	{
		return glm::inverseTranspose(mWorld);
	}

	void Transform::RebuildWorldMatrix()
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
