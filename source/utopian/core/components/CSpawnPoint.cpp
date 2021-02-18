#include "core/components/CSpawnPoint.h"
#include "core/components/CTransform.h"
#include <core/components/CPlayerControl.h>
#include <core/components/CRigidBody.h>
#include "core/components/Actor.h"
#include "core/Input.h"
#include "imgui/imgui.h"
#include "core/renderer/ImGuiRenderer.h"

namespace Utopian
{
	CSpawnPoint::CSpawnPoint(Actor* parent)
		: Component(parent)
	{
		SetName("CSpawnPoint");
	}

	CSpawnPoint::~CSpawnPoint()
	{
	}

	void CSpawnPoint::Update()
	{
		if (gInput().KeyPressed('R'))
		{
			Actor* playerActor = gWorld().GetPlayerActor();
			Transform& transform = playerActor->GetTransform();
			CRigidBody* rigidBody = playerActor->GetComponent<CRigidBody>();

			rigidBody->SetKinematic(true);
			transform.SetPosition(mTransform->GetPosition() + glm::vec3(0.0f, 0.0f, 0.0f));
			rigidBody->SetKinematic(false);

			CPlayerControl* playerControl = playerActor->GetComponent<CPlayerControl>();
			playerControl->StartLevelTimer();
		}
	}

	void CSpawnPoint::OnCreated()
	{
	}

	void CSpawnPoint::OnDestroyed()
	{
	}

	void CSpawnPoint::PostInit()
	{
		mTransform = GetParent()->GetComponent<CTransform>();
	}

	LuaPlus::LuaObject CSpawnPoint::GetLuaObject()
	{
		LuaPlus::LuaObject luaObject;
		luaObject.AssignNewTable(gLuaManager().GetLuaState());

		return luaObject;
	}
}