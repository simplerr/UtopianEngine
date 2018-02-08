#pragma once
#include <glm/glm.hpp>
#include "scene/Component.h"
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"

using namespace glm;

namespace Utopian
{
	class Actor;
	class CCamera;
	class CTransform;

	class COrbit : public Component
	{
	public:
		COrbit(Actor* parent, float speed);
		~COrbit();

		void Update() override;
		void OnCreated() override;

		// Setters
		void SetSpeed(float speed);
		void SetRadius(float radius);
		void SetTarget(const vec3& target);

		// Getters
		float GetSpeed() const;
		const vec3& GetTarget() const;

		// Type identification
		static uint32_t GetStaticType() {
			return Component::ComponentType::ORBIT;
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
		float mCounter;
	};
}
