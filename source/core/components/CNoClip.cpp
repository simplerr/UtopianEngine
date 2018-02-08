#include "core/components/CNoClip.h"
#include "core/components/CCamera.h"
#include "core/components/CTransform.h"
#include "core/components/Actor.h"
#include "Input.h"

namespace Utopian
{
	CNoClip::CNoClip(Actor* parent, float speed)
		: Component(parent)
	{
		SetSpeed(speed);
		SetSensitivity(0.2f);

		mCamera = GetParent()->GetComponent<CCamera>();
		mTransform = GetParent()->GetComponent<CTransform>();
	}

	CNoClip::~CNoClip()
	{
	}

	void CNoClip::Update()
	{
		if (gInput().KeyDown('W')) {
			vec3 dir = mCamera->GetDirection();
			mTransform->AddTranslation(mSpeed * dir);

		}
		if (gInput().KeyDown('S')) {
			vec3 dir = mCamera->GetDirection();
			mTransform->AddTranslation(mSpeed * -dir);

		}
		if (gInput().KeyDown('A')) {
			vec3 right = mCamera->GetRight();
			mTransform->AddTranslation(mSpeed * right);

		}
		if (gInput().KeyDown('D')) {
			vec3 right = mCamera->GetRight();
			mTransform->AddTranslation(mSpeed * -right);
		}

		if (gInput().KeyDown(VK_MBUTTON))
		{
			float deltaYaw = gInput().MouseDx() * mSensitivity;
			float deltaPitch = gInput().MouseDy() * mSensitivity;
			mCamera->AddOrientation(deltaYaw, deltaPitch);
		}
	}

	void CNoClip::OnCreated()
	{
	}

	void CNoClip::SetSpeed(float speed)
	{
		mSpeed = speed;
	}

	void CNoClip::SetSensitivity(float sensitivity)
	{
		mSensitivity = sensitivity;
	}

	float CNoClip::GetSpeed() const
	{
		return mSpeed;
	}

	float CNoClip::GetSensitivity() const
	{
		return mSensitivity;
	}
}