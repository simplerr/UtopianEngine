#include "Camera.h"
#include "vulkan/VulkanDebug.h"
#include "../external/glm/glm/gtc/matrix_transform.hpp"

namespace VulkanLib
{
	Camera::Camera()
	{
		mPosition = glm::vec3(7, 7, 7);
		mUp = glm::vec3(0, 1, 0);

		mYaw = mPitch = 0.0f;
		mFov = 60.0f;
		mNearPlane = 0.1f;
		mFarPlane = 256.0f;
		mAspectRatio = 4.0f / 3.0f;

		mLastX = mLastY = -1;
	}

	Camera::Camera(vec3 position, float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
	{
		this->mPosition = position;
		this->mFov = fieldOfView;
		this->mAspectRatio = aspectRatio;
		this->mNearPlane = nearPlane;
		this->mFarPlane = farPlane;
		mUp = glm::vec3(0, 1, 0);
		mYaw = mPitch = 0.0f;
		mLastX = mLastY = -1;
	}

	void Camera::Update()
	{
#if defined(_WIN32)

		if (GetAsyncKeyState('W')) {
			vec3 dir = GetDirection();
			mPosition += mSpeed * dir;

		}
		if (GetAsyncKeyState('S')) {
			vec3 dir = GetDirection();
			mPosition -= mSpeed * dir;

		}
		if (GetAsyncKeyState('A')) {
			vec3 right = GetRight();
			mPosition += mSpeed * right;

		}
		if (GetAsyncKeyState('D')) {
			vec3 right = GetRight();
			mPosition -= mSpeed * right;

		}

#endif
	}

#if defined(_WIN32)
	void Camera::HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{

		case WM_LBUTTONDOWN:
		{
			mLastX = LOWORD(lParam);
			mLastY = HIWORD(lParam);
			break;
		}
		case WM_MOUSEMOVE:
		{
			if (wParam & MK_LBUTTON)
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);

				if (mLastX == -1 && mLastY == -1) {
					mLastX = x;
					mLastY = y;
					break;
				}

				float dx = x - mLastX;
				float dy = mLastY - y;		// Other way around

				mYaw += dx * mSensitivity;
				mPitch += (dy * mSensitivity);

				CapAngles();

				mLastX = x;
				mLastY = y;
			}

			break;
		}
		case WM_KEYDOWN:
			if (wParam == VK_SPACE)
			{
				VulkanLib::VulkanDebug::ConsolePrint(mPosition, "Camera pos: ");
			}
			break;
		default:
			break;
		}
	}
#endif

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

	vec3 Camera::GetPosition()
	{
		return mPosition;
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
		return GetOrientation() * glm::translate(mat4(), mPosition);
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
		vec3 dir = glm::normalize(target - mPosition);
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