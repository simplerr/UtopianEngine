#include "PhysicsComponent.h"

namespace ECS
{
	PhysicsComponent::PhysicsComponent()
		: Component(PHYSICS_COMPONENT)
	{
		mVelocity = vec3(0.0f);
		mRotationSpeed = vec3(0.0f);
		mScaleSpeed = vec3(0.0f);
	}
	void PhysicsComponent::AddVelocity(vec3 delta)
	{
		mVelocity += delta;
	}

	void PhysicsComponent::AddRotationSpeed(vec3 delta)
	{
		mRotationSpeed += delta;
	}

	void PhysicsComponent::AddScaleSpeed(vec3 delta)
	{
		mScaleSpeed += delta;
	}

	void PhysicsComponent::SetVelocity(vec3 velocity)
	{
		mVelocity = velocity;
	}

	void PhysicsComponent::SetRotationSpeed(vec3 rotationSpeed)
	{
		mRotationSpeed = rotationSpeed;
	}

	void PhysicsComponent::SetScaleSpeed(vec3 scaleSpeed)
	{
		mScaleSpeed = scaleSpeed;
	}

	vec3 PhysicsComponent::GetVelocity()
	{
		return mVelocity;
	}

	vec3 PhysicsComponent::GetRotationSpeed()
	{
		return mRotationSpeed;
	}

	vec3 PhysicsComponent::GetScaleSpeed()
	{
		return mScaleSpeed;
	}
}