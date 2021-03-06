#include "core/components/CFinishPoint.h"
#include "core/components/CTransform.h"
#include <core/components/CPlayerControl.h>
#include "core/components/Actor.h"
#include "core/Input.h"
#include "imgui/imgui.h"
#include "core/renderer/ImGuiRenderer.h"

namespace Utopian
{
   CFinishPoint::CFinishPoint(Actor* parent)
      : Component(parent)
   {
      SetName("CFinishPoint");
   }

   CFinishPoint::~CFinishPoint()
   {
   }

   void CFinishPoint::Update()
   {
      Actor* playerActor = gWorld().GetPlayerActor();
      Transform& playerTransform = playerActor->GetTransform();

      const float intersectDistance = 1.0f;
      if (glm::distance(mTransform->GetPosition(), playerTransform.GetPosition()) < intersectDistance)
      {
         CPlayerControl* playerControl = playerActor->GetComponent<CPlayerControl>();
         playerControl->StopLevelTimer();
      }
   }

   void CFinishPoint::OnCreated()
   {
   }

   void CFinishPoint::OnDestroyed()
   {
   }

   void CFinishPoint::PostInit()
   {
      mTransform = GetParent()->GetComponent<CTransform>();
   }

   LuaPlus::LuaObject CFinishPoint::GetLuaObject()
   {
      LuaPlus::LuaObject luaObject;
      luaObject.AssignNewTable(gLuaManager().GetLuaState());

      return luaObject;
   }
}