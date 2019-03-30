#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "Transform.h"

namespace Utopian
{
	Transform::Transform(const glm::vec3& position)
	{
		SetPosition(position);
		SetRotation(glm::vec3(0, 0, 0));
		SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
	}

	Transform::Transform()
	{
		SetPosition(glm::vec3(0.0));
		SetRotation(glm::vec3(0.0));
		SetScale(glm::vec3(1.0));
	}

	Transform::~Transform()
	{
	}

	void Transform::SetPosition(const glm::vec3& position)
	{
		mPosition = position;
		RebuildWorldMatrix();
	}

	void Transform::SetRotation(const glm::vec3& rotation)
	{
		mRotation = rotation;
		RebuildWorldMatrix();
	}

	void Transform::SetScale(const glm::vec3& scale)
	{
		mScale = scale;
		RebuildWorldMatrix();
	}

	void Transform::SetQuaternion(const glm::quat& quaternion)
	{
		mQuaternion = quaternion;
	}

	void Transform::AddTranslation(const glm::vec3& translation)
	{
		mPosition += translation;
	}

	void Transform::AddRotation(float x, float y, float z)
	{
		mRotation += glm::vec3(x, y, z);
		RebuildWorldMatrix();
	}
	
	void Transform::AddRotation(const glm::vec3& rotation)
	{
		mRotation += rotation;
		RebuildWorldMatrix();
	}

	void Transform::AddScale(float x, float y, float z)
	{
		mScale += glm::vec3(x, y, z);
		RebuildWorldMatrix();
	}

	void Transform::AddScale(const glm::vec3& scale)
	{
		mScale += scale;
		RebuildWorldMatrix();
	}

	const glm::vec3& Transform::GetPosition() const
	{
		return mPosition;
	}

	const glm::vec3& Transform::GetRotation() const
	{
		return mRotation;
	}

	const glm::vec3& Transform::GetScale() const
	{
		return mScale;
	}

	const glm::mat4& Transform::GetWorldMatrix() const
	{
		return mWorld;
	}

	const glm::mat4& Transform::GetWorldInverseTransposeMatrix() const
	{
		return glm::inverseTranspose(mWorld);
	}

	void Transform::RebuildWorldMatrix()
	{
		glm::mat4 world;

		//world = glm::translate(world, mPosition);
		//world = glm::rotate(world, glm::radians(mRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		//world = glm::rotate(world, glm::radians(mRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		//world = glm::rotate(world, glm::radians(mRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		//world = glm::scale(world, mScale);

		glm::mat4 translation = glm::translate(glm::mat4(), mPosition);
		//glm::mat4 rotation = glm::rotate(glm::mat4(), glm::radians(mRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		//rotation = glm::rotate(rotation, glm::radians(mRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		//rotation = glm::rotate(rotation, glm::radians(mRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 rotation = glm::mat4_cast(mQuaternion);
		glm::mat4 scale = glm::scale(glm::mat4(), mScale);

		world = translation * rotation * scale;

		mWorld = world;
	}
}
