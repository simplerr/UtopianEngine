#pragma once
#include <glm/glm.hpp>
#include "scene/SceneComponent.h"
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"

using namespace glm;

namespace Scene
{
	class SceneEntity;
	class CCamera;
	class CTransform;

	class COrbit : public SceneComponent
	{
	public:
		COrbit(SceneEntity* parent, float speed);
		~COrbit();

		void Update() override;
		void OnCreated() override;

		// Setters
		void SetSpeed(float speed);
		void SetRadius(float radius);
		void SetTarget(const vec3& target);

		// Getters
		float GetSpeed() const;

		// Type identification
		static uint32_t GetStaticType() {
			return SceneComponent::ComponentType::FREE_CAMERA;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		CCamera* mCamera; // For convenience
		CTransform* mTransform;
		vec3 mTarget;
		float mSpeed;
		float mRadius;
	};
}
