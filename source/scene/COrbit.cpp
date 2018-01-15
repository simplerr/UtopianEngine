#include "scene/COrbit.h"
#include "scene/CTransform.h"
#include "scene/CCamera.h"
#include "scene/SceneEntity.h"

namespace Scene
{
	COrbit::COrbit(SceneEntity* parent, float speed)
		: SceneComponent(parent)
	{
		mTransform = GetParent()->GetComponent<CTransform>();
		mCamera = GetParent()->GetComponent<CCamera>();
		SetSpeed(speed);
		SetRadius(15000);
		SetTarget(mTransform->GetPosition());
	}

	COrbit::~COrbit()
	{
	}

	void COrbit::Update()
	{
		static float counter = 0.0f;

		float x = cosf(counter) * mRadius;
		float z = sinf(counter) * mRadius;

		mTransform->SetTransform(vec3(mTarget.x + x, mTransform->GetPosition().y, mTarget.z + z));
		mCamera->LookAt(mTarget);

		counter += mSpeed;
	}

	void COrbit::OnCreated()
	{
	}

	void COrbit::SetSpeed(float speed)
	{
		mSpeed = speed;
	}

	void COrbit::SetRadius(float radius)
	{
		mRadius = radius;
	}

	void COrbit::SetTarget(const vec3& target)
	{
		mTarget = target;
	}

	float COrbit::GetSpeed() const
	{
		return mSpeed;
	}
}