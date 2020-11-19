#include "core/components/CNoClip.h"
#include "core/components/CCamera.h"
#include "core/components/CTransform.h"
#include "core/components/Actor.h"
#include "core/Input.h"

namespace Utopian
{
	CNoClip::CNoClip(Actor* parent, float speed)
		: Component(parent)
	{
		SetName("CNoClip");
		SetSpeed(speed);
		SetSensitivity(0.2f);
	}

	CNoClip::~CNoClip()
	{
	}

	void CNoClip::Update()
	{
		if (gInput().KeyDown('W')) {
			glm::vec3 dir = mCamera->GetDirection();
			mTransform->AddTranslation(mSpeed * dir);

		}
		if (gInput().KeyDown('S')) {
			glm::vec3 dir = mCamera->GetDirection();
			mTransform->AddTranslation(mSpeed * -dir);

		}
		if (gInput().KeyDown('A')) {
			glm::vec3 right = mCamera->GetRight();
			mTransform->AddTranslation(mSpeed * right);

		}
		if (gInput().KeyDown('D')) {
			glm::vec3 right = mCamera->GetRight();
			mTransform->AddTranslation(mSpeed * -right);
		}

		if (gInput().KeyPressed('J'))
			SetSpeed(10.0);
		else if (gInput().KeyPressed('K'))
			SetSpeed(0.04);

		// Todo: Mousewheel broken
		if (gInput().KeyDown(VK_RBUTTON))
		{
			float deltaYaw = gInput().MouseDx() * mSensitivity;
			float deltaPitch = gInput().MouseDy() * mSensitivity;
			mCamera->AddOrientation(deltaYaw, deltaPitch);
		}
	}

	void CNoClip::OnCreated()
	{
	}

	void CNoClip::OnDestroyed()
	{
	}

	void CNoClip::PostInit()
	{
		mCamera = GetParent()->GetComponent<CCamera>();
		mTransform = GetParent()->GetComponent<CTransform>();
	}

	LuaPlus::LuaObject CNoClip::GetLuaObject()
	{
		LuaPlus::LuaObject luaObject;
		luaObject.AssignNewTable(gLuaManager().GetLuaState());

		luaObject.SetNumber("speed", GetSpeed());

		return luaObject;
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