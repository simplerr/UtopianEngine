#include "core/components/CNoClip.h"
#include "core/components/CCamera.h"
#include "core/components/CTransform.h"
#include "core/components/Actor.h"
#include "core/Input.h"
#include "core/renderer/ImGuiRenderer.h"

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

   void CNoClip::Update(double deltaTime)
   {
      float speed = mSpeed * (float)deltaTime;

      if (gInput().KeyDown('W')) {
         glm::vec3 dir = mCamera->GetDirection();
         mTransform->AddTranslation(speed * dir);

      }
      if (gInput().KeyDown('S')) {
         glm::vec3 dir = mCamera->GetDirection();
         mTransform->AddTranslation(speed * -dir);

      }
      if (gInput().KeyDown('A')) {
         glm::vec3 right = mCamera->GetRight();
         mTransform->AddTranslation(speed * right);

      }
      if (gInput().KeyDown('D')) {
         glm::vec3 right = mCamera->GetRight();
         mTransform->AddTranslation(speed * -right);
      }

      // Todo: Mousewheel broken
      if (ImGuiRenderer::GetMode() == UiMode::UI_MODE_GAME || gInput().KeyDown(VK_RBUTTON))
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