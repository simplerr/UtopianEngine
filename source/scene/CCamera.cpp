#include "scene/CCamera.h"
#include "scene/World.h"
#include "Camera.h"

namespace Scene
{
	CCamera::CCamera(SceneEntity* parent)
		: SceneComponent(parent)
	{

	}

	CCamera::~CCamera()
	{

	}

	void CCamera::Update()
	{

	}

	void CCamera::OnCreated()
	{
		mInternal = Vulkan::Camera::Create();

		World::Instance().BindNode(mInternal, GetParent());
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