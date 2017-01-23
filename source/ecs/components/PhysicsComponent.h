#pragma once

#include <glm/glm.hpp>
#include "Component.h"

using namespace glm;

namespace ECS
{
	class PhysicsComponent : public Component
	{
	public:
		PhysicsComponent();

		void AddVelocity(vec3 delta);
		void AddRotationSpeed(vec3 delta);
		void AddScaleSpeed(vec3 delta);

		void SetVelocity(vec3 velocity);
		void SetRotationSpeed(vec3 rotationSpeed);
		void SetScaleSpeed(vec3 scaleSpeed);

		vec3 GetVelocity();
		vec3 GetRotationSpeed();
		vec3 GetScaleSpeed();
	public:
		vec3 mVelocity;
		vec3 mRotationSpeed;
		vec3 mScaleSpeed;
	};
}
