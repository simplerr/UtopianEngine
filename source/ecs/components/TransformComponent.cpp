#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "TransformComponent.h"

namespace ECS
{
	TransformComponent::TransformComponent(vec3 position)
		: Component(Type::TRANSFORM_COMPONENT)
	{
		SetPosition(position);
		SetRotation(vec3(0, 0, 0));
		SetScale(vec3(1.0f, 1.0f, 1.0f));
	}

	TransformComponent::~TransformComponent()
	{
	}

	void TransformComponent::SetPosition(vec3 position)
	{
		mPosition = position;
		RebuildWorldMatrix();
	}

	void TransformComponent::SetRotation(vec3 rotation)
	{
		mRotation = rotation;
		RebuildWorldMatrix();
	}

	void TransformComponent::SetScale(vec3 scale)
	{
		mScale = scale;
		RebuildWorldMatrix();
	}

	void TransformComponent::AddRotation(float x, float y, float z)
	{
		mRotation += vec3(x, y, z);
		RebuildWorldMatrix();
	}

	vec3 TransformComponent::GetPosition()
	{
		return mPosition;
	}

	vec3 TransformComponent::GetRotation()
	{
		return mRotation;
	}

	vec3 TransformComponent::GetScale()
	{
		return mScale;
	}

	mat4 TransformComponent::GetWorldMatrix()
	{
		return mWorld;
	}

	mat4 TransformComponent::GetWorldInverseTransposeMatrix()
	{
		return glm::inverseTranspose(mWorld);
	}

	void TransformComponent::RebuildWorldMatrix()
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