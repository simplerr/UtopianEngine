#include "Camera.h"
#include "Window.h"
#include "vulkan/VulkanDebug.h"
#include "../external/glm/glm/gtc/matrix_transform.hpp"
#include "Input.h"
#include "scene/SceneRenderer.h"

namespace Vulkan
{
	Camera::Camera(Window* window, vec3 position, float fieldOfView, float nearPlane, float farPlane)
	{
		SetPosition(position);
		this->mFov = fieldOfView;
		this->mAspectRatio = (float)window->GetWidth() / (float)window->GetHeight();
		this->mNearPlane = nearPlane;
		this->mFarPlane = farPlane;
		mUp = glm::vec3(0, 1, 0);
		mYaw = mPitch = 0.0f;
		mWindow = window;
	}

	SharedPtr<Camera> Camera::Create(Window* window, vec3 position, float fieldOfView, float nearPlane, float farPlane)
	{
		SharedPtr<Camera> instance(new Camera(window, position, fieldOfView, nearPlane, farPlane));
		instance->Initialize();

		return instance;
	}

	void Camera::SetMainCamera()
	{
		Scene::SceneRenderer::Instance().SetMainCamera(this);
	}

	void Camera::Initialize()
	{
		Scene::SceneRenderer::Instance().AddCamera(this);
	}

	void Camera::SetFov(float fov)
	{
		mFov = fov;
	}

	void Camera::SetNearPlane(float nearPlane)
	{
		mNearPlane = nearPlane;
	}

	void Camera::SetFarPlane(float farPlane)
	{
		mFarPlane = farPlane;
	}

	void Camera::SetWindow(Vulkan::Window* window)
	{
		mWindow = window;
	}

	void Camera::SetAspectRatio(float aspectRatio)
	{
		mAspectRatio = aspectRatio;
	}

	Ray Camera::GetPickingRay()
	{
		// Camera/view matrix
		mat4 viewMatrix = GetView();
		mat4 projectionMatrix = GetProjection();

		mat4 inverseView = glm::inverse(viewMatrix);
		mat4 inverseProjection = glm::inverse(projectionMatrix);

		vec2 cursorPos = gInput().GetMousePosition();

		float vx = (+2.0f * cursorPos.x / mWindow->GetWidth() - 1.0f);
		float vy = (-2.0f * cursorPos.y / mWindow->GetHeight() + 1.0f);

		vec4 rayDir = inverseProjection * vec4(-vx, vy, 1.0, 1.0);
		rayDir.z = 1.0;
		rayDir.w = 0;
		rayDir = inverseView * rayDir;
		vec3 rayFinalDir = glm::normalize(vec3(rayDir.x, rayDir.y, rayDir.z));

		return Vulkan::Ray(GetPosition(), rayFinalDir);
	}

	vec3 Camera::GetDirection()
	{
		vec4 forward = glm::inverse(GetOrientation()) * vec4(0, 0, 1, 1);
		vec3 f = forward;
		if (f.y != -1.0f)
			int a = 1;
		f = glm::normalize(f);
		return f;
	}

	vec3 Camera::GetRight()
	{
		vec4 right = glm::inverse(GetOrientation()) * vec4(1, 0, 0, 1);
		vec3 r = right;
		r = glm::normalize(r);
		return r;
	}

	vec3 Camera::GetTarget()
	{
		return GetPosition() + GetDirection();
	}

	vec3 Camera::GetUp()
	{
		return mUp;
	}

	mat4 Camera::GetOrientation()
	{
		mat4 orientation = mat4();
		orientation = glm::rotate(orientation, glm::radians(mPitch), vec3(1, 0, 0));		// Pitch (vertical angle)
		orientation = glm::rotate(orientation, glm::radians(mYaw), vec3(0, 1, 0));			// Yaw (horizontal angle)
		orientation = glm::rotate(orientation, glm::radians(0.0f), vec3(0, 0, 1));			// Yaw (horizontal angle)
		return orientation;
	}

	void Camera::AddOrientation(float yaw, float pitch)
	{
		this->mYaw += yaw;
		this->mPitch += pitch;
		CapAngles();
	}

	void Camera::SetOrientation(float yaw, float pitch)
	{
		this->mYaw = yaw;
		this->mPitch = pitch;
		CapAngles();
	}

	mat4 Camera::GetView()
	{
		//mat4 viewMatrix = glm::lookAt(GetPosition(), GetTarget(), GetUp());
		//return viewMatrix;
		return GetOrientation() * glm::translate(mat4(), GetPosition());
	}

	mat4 Camera::GetProjection()
	{
		return glm::perspective(glm::radians(mFov), mAspectRatio, mNearPlane, mFarPlane);
	}

	mat4 Camera::GetMatrix()
	{
		return GetProjection() * GetView();
	}

	void Camera::LookAt(vec3 target)
	{
		vec3 dir = glm::normalize(target - GetPosition());
		mPitch = glm::degrees(asinf(dir.y));
		mYaw = -glm::degrees(atan2f(dir.x, dir.z));		// Note the - signs
	}

	void Camera::CapAngles()
	{
		mYaw = fmodf(mYaw, 360.0f);

		if (mYaw < 0.0f)
			mYaw += 360.0f;

		if (mPitch > 85.0f)
			mPitch = 85.0;
		else if (mPitch < -85.0)
			mPitch = -85.0;
	}

	float Camera::GetPitch()
	{
		return mPitch;
	}

	float Camera::GetYaw()
	{
		return mYaw;
	}
}	// VulkanLib namespace