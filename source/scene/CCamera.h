#pragma once
#include <glm/glm.hpp>
#include "scene/SceneComponent.h"
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"

using namespace glm;

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
		void LookAt(const vec3& target);
		void AddOrientation(float yaw, float pitch);
		void SetOrientation(float yaw, float pitch);

		// Getters
		const vec3& GetDirection() const;
		const vec3& GetTarget() const;
		const vec3& GetRight() const;
		const vec3& GetUp() const;
		float GetPitch() const;
		float GetYaw() const;


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
