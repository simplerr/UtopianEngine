#pragma once
#include <glm/glm.hpp>
#include "scene/SceneComponent.h"
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"

using namespace glm;

namespace Scene
{
	class Actor;
	class CCamera;
	class CTransform;

	class CNoClip : public SceneComponent
	{
	public:
		CNoClip(Actor* parent, float speed);
		~CNoClip();

		void Update() override;
		void OnCreated() override;

		// Setters
		void SetSpeed(float speed);
		void SetSensitivity(float sensitivity);

		// Getters
		float GetSpeed() const;
		float GetSensitivity() const;

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
		float mSensitivity;
		float mSpeed;
	};
}
