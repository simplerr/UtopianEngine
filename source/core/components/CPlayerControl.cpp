#include "core/components/COrbit.h"
#include "core/components/CNoClip.h"
#include "core/components/CTransform.h"
#include "core/components/CCamera.h"
#include "core/components/CPlayerControl.h"
#include "core/components/Actor.h"
#include "Input.h"

namespace Utopian
{
	CPlayerControl::CPlayerControl(Actor* parent)
		: Component(parent)
	{
		SetName("CPlayerControl");
	}

	CPlayerControl::~CPlayerControl()
	{
	}

	void CPlayerControl::Update()
	{
		/*if (gInput().KeyPressed('R'))
		{
			mOrbit->SetActive(true);
			mNoClip->SetActive(false);

			mOrbit->SetRadius(glm::distance(mTransform->GetPosition(), mOrbit->GetTarget()));
		}
		else if (gInput().KeyPressed('T'))
		{
			mOrbit->SetActive(false);
			mNoClip->SetActive(true);
		}*/
	}

	void CPlayerControl::PostInit()
	{
		mTransform = GetParent()->GetComponent<CTransform>();
		mCamera = GetParent()->GetComponent<CCamera>();
		mOrbit = GetParent()->GetComponent<COrbit>();
		mNoClip = GetParent()->GetComponent<CNoClip>();

		if (mOrbit != nullptr)
			mOrbit->SetActive(false);
	}

	LuaPlus::LuaObject CPlayerControl::GetLuaObject()
	{
		LuaPlus::LuaObject luaObject;
		luaObject.AssignNewTable(gLuaManager().GetLuaState());

		luaObject.SetNumber("empty", 0.0f);

		return luaObject;
	}
}