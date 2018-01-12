#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include "Platform.h"
#include "Collision.h"
#include "utility/Common.h"
#include "scene/SceneNode.h"

using namespace glm;

namespace Vulkan
{
	class Window;

	class Camera : public Scene::SceneNode
	{
	public:
		Camera();
		Camera(Window* window, vec3 position, float fieldOfView, float nearPlane, float farPlane);

		static SharedPtr<Camera> Create();
		void Initialize();

		void Update();

		Ray GetPickingRay();

		vec3 GetDirection();

		// New
		mat4 GetOrientation();
		mat4 GetView();
		mat4 GetProjection();
		mat4 GetMatrix();
		vec3 GetRight();
		vec3 GetPosition();
		vec3 GetTarget();
		vec3 GetUp();
		float GetPitch();
		float GetYaw();
		void AddOrientation(float yaw, float pitch);
		void SetOrientation(float yaw, float pitch);
		void SetPosition(glm::vec3 position);
		void LookAt(vec3 target);
		void CapAngles();

		// [NOTE][HACK] Vulkan & OpenGL have different pitch movement
		int hack = 1;

		vec3 mPosition;
	private:
		Window* mWindow;
		vec3 mUp;

		float mPitch;	// Vertical angle
		float mYaw;		// Horizontal angle
		float mFov;
		float mNearPlane;
		float mFarPlane;
		float mAspectRatio;

		float mSensitivity = 0.2f;
		float mSpeed = 100.5f;

		int mLastX, mLastY;
	};
}	// VulkanLib namespace