#pragma once
#include "scene/SceneComponent.h"
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"

namespace Scene
{
	class SceneEntity;

	class CCamera : public SceneComponent
	{
	public:
		CCamera(SceneEntity* parent);
		~CCamera();

		void Update() override;
		void OnCreated() override;

		// Setters

		// Getters

		// Type identification
		static uint32_t GetStaticType() {
			return SceneComponent::ComponentType::CAMERA;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		SharedPtr<Vulkan::Camera> mInternal;
	};
}
