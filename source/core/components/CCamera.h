#pragma once
#include <glm/glm.hpp>
#include "core/components/Component.h"
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"
#include "core/LuaManager.h"

using namespace glm;

namespace Utopian
{
	class Actor;

	class CCamera : public Component
	{
	public:
		CCamera(Actor* parent, Utopian::Window* window, float fieldOfView, float nearPlane, float farPlane);
		~CCamera();

		void Update() override;
		void OnCreated() override;
		void OnDestroyed() override;
		void PostInit() override;

		LuaPlus::LuaObject GetLuaObject() override;

		// Setters
		void LookAt(const vec3& target);
		void AddOrientation(float yaw, float pitch);
		void SetOrientation(float yaw, float pitch);
		void SetFov(float fov);
		void SetNearPlane(float nearPlane);
		void SetFarPlane(float farPlane);
		void SetAspectRatio(float aspectRatio);
		void SetWindow(Utopian::Window* window);
		void SetMainCamera();

		// Getters
		const vec3& GetDirection() const;
		const vec3& GetTarget() const;
		const vec3& GetRight() const;
		const vec3& GetUp() const;
		const vec3& GetLookAt() const;
		float GetPitch() const;
		float GetYaw() const;
		float GetFov() const;
		float GetNearPlane() const;
		float GetFarPlane() const;

		// Type identification
		static uint32_t GetStaticType() {
			return Component::ComponentType::CAMERA;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		SharedPtr<Utopian::Camera> mInternal;
	};
}
