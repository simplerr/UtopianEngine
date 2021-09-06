#include "core/components/CBloomLight.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/CLight.h"
#include "core/components/Actor.h"
#include "imgui/imgui.h"

namespace Utopian
{
   CBloomLight::CBloomLight(Actor* parent)
      : Component(parent)
   {
      SetName("CBloomLight");
   }

   CBloomLight::~CBloomLight()
   {
   }

   void CBloomLight::Update()
   {
      float renderableBrightness = mRenderable->GetColor().w;
      glm::vec4 lightColor = mLight->GetLightColor().ambient;
      mRenderable->SetColor(glm::vec4(lightColor.r, lightColor.g, lightColor.b, renderableBrightness));
   }

   void CBloomLight::OnCreated()
   {
   }

   void CBloomLight::OnDestroyed()
   {
   }

   void CBloomLight::PostInit()
   {
      mTransform = GetParent()->GetComponent<CTransform>();
      mRenderable = GetParent()->GetComponent<CRenderable>();
      mLight = GetParent()->GetComponent<CLight>();
   }

   LuaPlus::LuaObject CBloomLight::GetLuaObject()
   {
      LuaPlus::LuaObject luaObject;
      luaObject.AssignNewTable(gLuaManager().GetLuaState());

      luaObject.SetNumber("empty", 0.0f);

      return luaObject;
   }
}