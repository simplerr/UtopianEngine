#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

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
		void AddScale(float x, float y, float z);
		void AddScale(vec3 scale);

		vec3 GetPosition();
		vec3 GetRotation();
		vec3 GetScale();
		mat4 GetWorldMatrix();
		mat4 GetWorldInverseTransposeMatrix();

		void RebuildWorldMatrix();
	private:

		mat4 mWorld;
		vec3 mPosition;
		vec3 mRotation;
		vec3 mScale;
	};
}
