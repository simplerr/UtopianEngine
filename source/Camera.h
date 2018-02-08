#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include "Platform.h"
#include "Collision.h"
#include "utility/Common.h"
#include "scene/SceneNode.h"
#include "vulkan/VulkanInclude.h"

using namespace glm;

namespace Utopian::Vk
{
	class Window;

	class Camera : public Utopian::SceneNode
	{
	public:
		Camera(Utopian::Window* window, vec3 position, float fieldOfView, float nearPlane, float farPlane);

		static SharedPtr<Camera> Create(Utopian::Window* window, vec3 position, float fieldOfView, float nearPlane, float farPlane);
		void Initialize();

		void SetFov(float fov);
		void SetNearPlane(float nearPlane);
		void SetFarPlane(float farPlane);
		void SetWindow(Utopian::Window* window);
		void SetAspectRatio(float aspectRatio);

		Ray GetPickingRay();

		vec3 GetDirection();

		// New
		mat4 GetOrientation();
		mat4 GetView();
		mat4 GetProjection();
		mat4 GetMatrix();
		vec3 GetRight();
		vec3 GetTarget();
		vec3 GetUp();
		float GetPitch();
		float GetYaw();
		void AddOrientation(float yaw, float pitch);
		void SetOrientation(float yaw, float pitch);
		void LookAt(vec3 target);
		void CapAngles();

		void SetMainCamera();

		// [NOTE][HACK] Vulkan & OpenGL have different pitch movement
		int hack = 1;
	private:
		Utopian::Window* mWindow;
		vec3 mUp;

		float mPitch;	// Vertical angle
		float mYaw;		// Horizontal angle
		float mFov;
		float mNearPlane;
		float mFarPlane;
		float mAspectRatio;
	};
}	// VulkanLib namespace