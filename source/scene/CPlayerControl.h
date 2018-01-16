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
	class COrbit;
	class CNoClip;

	class CPlayerControl : public SceneComponent
	{
	public:
		CPlayerControl(SceneEntity* parent);
		~CPlayerControl();

		void Update() override;

		// Type identification
		static uint32_t GetStaticType() {
			return SceneComponent::ComponentType::FREE_CAMERA;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		CCamera* mCamera; // For convenience
		CNoClip* mNoClip;
		COrbit* mOrbit;
		CTransform* mTransform;
		vec3 mTarget;
		float mSpeed;
		float mRadius;
		float mCounter;
	};
}
