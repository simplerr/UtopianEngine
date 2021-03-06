#include "core/components/CCamera.h"
#include "core/World.h"
#include "core/components/Actor.h"
#include "core/Camera.h"

namespace Utopian
{
   CCamera::CCamera(Actor* parent, Utopian::Window* window, float fieldOfView, float nearPlane, float farPlane)
      : Component(parent)
   {
      SetName("CCamera");

      mInternal = Camera::Create(window, glm::vec3(0, 0, 0), fieldOfView, nearPlane, farPlane);
      World::Instance().BindNode(mInternal, GetParent());
   }

   CCamera::~CCamera()
   {

   }

   void CCamera::Update()
   {

   }

   void CCamera::OnCreated()
   {

   }

   void CCamera::OnDestroyed()
   {
      // Todo:
      assert(0);
      World::Instance().RemoveNode(mInternal);
   }

   void CCamera::PostInit()
   {
      auto transform = GetParent()->GetTransform();
      mInternal->SetPosition(transform.GetPosition());
   }

   LuaPlus::LuaObject CCamera::GetLuaObject()
   {
      LuaPlus::LuaObject luaObject;
      luaObject.AssignNewTable(gLuaManager().GetLuaState());

      luaObject.SetNumber("fov", GetFov());
      luaObject.SetNumber("near_plane", GetNearPlane());
      luaObject.SetNumber("far_plane", GetFarPlane());
      luaObject.SetNumber("look_at_x", GetLookAt().x);
      luaObject.SetNumber("look_at_y", GetLookAt().y);
      luaObject.SetNumber("look_at_z", GetLookAt().z);

      return luaObject;
   }

   void CCamera::LookAt(const glm::vec3& target)
   {
      mInternal->LookAt(target);
   }

   void CCamera::AddOrientation(float yaw, float pitch)
   {
      mInternal->AddOrientation(yaw, pitch);
   }

   void CCamera::SetOrientation(float yaw, float pitch)
   {
      mInternal->SetOrientation(yaw, pitch);
   }

   void CCamera::SetFov(float fov)
   {
      mInternal->SetFov(fov);
   }

   void CCamera::SetNearPlane(float nearPlane)
   {
      mInternal->SetNearPlane(nearPlane);
   }

   void CCamera::SetFarPlane(float farPlane)
   {
      mInternal->SetFarPlane(farPlane);
   }

   void CCamera::SetAspectRatio(float aspectRatio)
   {
      mInternal->SetAspectRatio(aspectRatio);
   }

   void CCamera::SetWindow(Utopian::Window* window)
   {
      mInternal->SetWindow(window);
   }
   
   void CCamera::SetMainCamera()
   {
      mInternal->SetMainCamera();
   }

   glm::vec3 CCamera::GetDirection() const
   {
      return mInternal->GetDirection();
   }

   glm::vec3 CCamera::GetTarget() const
   {
      return mInternal->GetTarget();
   }

   glm::vec3 CCamera::GetRight() const
   {
      return mInternal->GetRight();
   }

   glm::vec3 CCamera::GetUp() const
   {
      return mInternal->GetUp();
   }

   glm::vec3 CCamera::GetLookAt() const
   {
      return mInternal->GetLookAt();
   }

   float CCamera::GetPitch() const
   {
      return mInternal->GetPitch();
   }

   float CCamera::GetYaw() const
   {
      return mInternal->GetYaw();
   }

   float CCamera::GetFov() const
   {
      return mInternal->GetFov();
   }

   float CCamera::GetNearPlane() const
   {
      return mInternal->GetNearPlane();
   }

   float CCamera::GetFarPlane() const
   {
      return mInternal->GetFarPlane();
   }
}