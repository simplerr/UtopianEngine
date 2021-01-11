#include "core/components/COrbit.h"
#include "core/components/CTransform.h"
#include "core/components/CCamera.h"
#include "core/components/Actor.h"
#include "imgui/imgui.h"

namespace Utopian
{
	COrbit::COrbit(Actor* parent, float speed)
		: Component(parent)
	{
		SetSpeed(speed);
		SetRadius(15000);

		// Random orbit position
		mCounter = (float)(rand() % 100);
	}

	COrbit::~COrbit()
	{
	}

	void COrbit::Update()
	{
		float x = cosf(mCounter) * mRadius;
		float z = sinf(mCounter) * mRadius;

		mTransform->SetPosition(glm::vec3(mTarget.x + x, mTransform->GetPosition().y, mTarget.z + z));

		if (mCamera != nullptr)
			mCamera->LookAt(mTarget);

		mCounter += mSpeed;
	}

	void COrbit::OnCreated()
	{
	}

	void COrbit::OnDestroyed()
	{
	}

	void COrbit::PostInit()
	{
		mTransform = GetParent()->GetComponent<CTransform>();
		mCamera = GetParent()->GetComponent<CCamera>();
		SetTarget(mTransform->GetPosition());
	}

	void COrbit::SetSpeed(float speed)
	{
		mSpeed = speed;
	}

	void COrbit::SetRadius(float radius)
	{
		mRadius = radius;
	}

	void COrbit::SetTarget(const glm::vec3& target)
	{
		mTarget = target;
	}

	float COrbit::GetSpeed() const
	{
		return mSpeed;
	}

	const glm::vec3& COrbit::GetTarget() const
	{
		return mTarget;
	}
}