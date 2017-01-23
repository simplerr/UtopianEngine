#pragma once

#include <glm/glm.hpp>
#include "Component.h"

using namespace glm;

namespace ECS
{
	class TransformComponent : public Component
	{
	public:
		TransformComponent(vec3 position);
		~TransformComponent();

		void SetPosition(vec3 position);
		void SetRotation(vec3 rotation);
		void SetScale(vec3 scale);

		void AddRotation(float x, float y, float z);

		vec3 GetPosition();
		vec3 GetRotation();
		vec3 GetScale();
		mat4 GetWorldMatrix();
		mat4 GetWorldInverseTransposeMatrix();

	private:
		void RebuildWorldMatrix();

		mat4 mWorld;
		vec3 mPosition;
		vec3 mRotation;
		vec3 mScale;
	};
}
