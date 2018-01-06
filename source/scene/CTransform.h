#pragma once
#include <glm/glm.hpp>
#include "SceneComponent.h"

using namespace glm;

namespace Scene
{
	class SceneEntity;

	class CTransform : public SceneComponent
	{
	public:
		CTransform(SceneEntity* parent, vec3 position);
		~CTransform();

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

		// Type identification
		static uint32_t GetStaticType() {
			return SceneComponent::ComponentType::TRANSFORM;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:

		mat4 mWorld;
		vec3 mPosition;
		vec3 mRotation;
		vec3 mScale;
	};
}