#include "core/components/CRandomPaths.h"
#include "core/components/CTransform.h"
#include "core/components/Actor.h"
#include "core/Terrain.h"
#include "imgui/imgui.h"
#include "utility/math/Helpers.h"
#include <random>

namespace Utopian
{
   CRandomPaths::CRandomPaths(Actor* parent, Terrain* terrain)
      : Component(parent)
   {
      SetName("CRandomPaths");
      mTerrain = terrain;
      mSpeed = 1.0f;
      mTarget = GenerateNewTarget();
   }

   CRandomPaths::~CRandomPaths()
   {
   }

   void CRandomPaths::Update()
   {
      glm::vec2 pos = glm::vec2(mTransform->GetPosition().x, mTransform->GetPosition().z);
      glm::vec2 delta = glm::vec2(mTarget.x - pos.x, mTarget.y - pos.y);
      delta = glm::normalize(delta);

      glm::vec2 newPos = pos + delta * mSpeed;
      float height = mTerrain->GetHeight(newPos.x, newPos.y);
      mTransform->SetPosition(glm::vec3(newPos.x, height, newPos.y));

      if (glm::distance(newPos, mTarget) < 10.0f)
         mTarget = GenerateNewTarget();
   }

   glm::vec2 CRandomPaths::GenerateNewTarget()
   {
      float range = mTerrain->GetTerrainSize();

      glm::vec2 target = glm::vec2(Math::GetRandom(-range / 2.0f, range / 2.0f));
      return target;
   }

   void CRandomPaths::OnCreated()
   {
   }

   void CRandomPaths::OnDestroyed()
   {
   }

   void CRandomPaths::PostInit()
   {
      mTransform = GetParent()->GetComponent<CTransform>();
   }

   LuaPlus::LuaObject CRandomPaths::GetLuaObject()
   {
      LuaPlus::LuaObject luaObject;
      luaObject.AssignNewTable(gLuaManager().GetLuaState());

      luaObject.SetNumber("empty", 0.0f);

      return luaObject;
   }
}