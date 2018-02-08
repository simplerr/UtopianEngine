#include "scene/CCamera.h"
#include "scene/World.h"
#include "scene/Actor.h"
#include "Camera.h"

namespace Utopian
{
	CCamera::CCamera(Actor* parent, Utopian::Window* window, float fieldOfView, float nearPlane, float farPlane)
		: SceneComponent(parent)
	{
		mInternal = Utopian::Vk::Camera::Create(window, vec3(0, 0, 0), fieldOfView, nearPlane, farPlane);
		auto transform = GetParent()->GetTransform();
		mInternal->SetPosition(transform.GetPosition());

		World::Instance().BindNode(mInternal, GetParent());
	}

	CCamera::~CCamera()
	{

	}

	void CCamera::Update()
	{

	}

	void CCamera::OnCreated()
	{

	}

	void CCamera::LookAt(const vec3& target)
	{
		mInternal->LookAt(target);
	}

	void CCamera::AddOrientation(float yaw, float pitch)
	{
		mInternal->AddOrientation(yaw, pitch);
	}

	void CCamera::SetOrientation(float yaw, float pitch)
	{
		mInternal->SetOrientation(yaw, pitch);
	}

	void CCamera::SetFov(float fov)
	{
		mInternal->SetFov(fov);
	}

	void CCamera::SetNearPlane(float nearPlane)
	{
		mInternal->SetNearPlane(nearPlane);
	}

	void CCamera::SetFarPlane(float farPlane)
	{
		mInternal->SetFarPlane(farPlane);
	}

	void CCamera::SetAspectRatio(float aspectRatio)
	{
		mInternal->SetAspectRatio(aspectRatio);
	}

	void CCamera::SetWindow(Utopian::Window* window)
	{
		mInternal->SetWindow(window);
	}
	
	void CCamera::SetMainCamera()
	{
		mInternal->SetMainCamera();
	}

	const vec3& CCamera::GetDirection() const
	{
		return mInternal->GetDirection();
	}

	const vec3& CCamera::GetTarget() const
	{
		return mInternal->GetTarget();
	}

	const vec3& CCamera::GetRight() const
	{
		return mInternal->GetRight();
	}

	const vec3& CCamera::GetUp() const
	{
		return mInternal->GetUp();
	}

	float CCamera::GetPitch() const
	{
		return mInternal->GetPitch();
	}

	float CCamera::GetYaw() const
	{
		return mInternal->GetYaw();
	}
}