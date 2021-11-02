#include "core/components/CLight.h"
#include "core/SceneNode.h"
#include "core/World.h"
#include "imgui/imgui.h"

namespace Utopian
{
   CLight::CLight(Actor* parent)
      : Component(parent)
   {
      SetName("CLight");
   }

   CLight::~CLight()
   {

   }

   void CLight::Update()
   {
   }

   void CLight::OnCreated()
   {
      mInternal = Light::Create();

      World::Instance().BindNode(mInternal, GetParent());
   }

   void CLight::OnDestroyed()
   {
      mInternal->OnDestroyed();
      World::Instance().RemoveNode(mInternal);
   }

   void CLight::PostInit()
   {
   }

   LuaPlus::LuaObject CLight::GetLuaObject()
   {
      LuaPlus::LuaObject luaObject;
      luaObject.AssignNewTable(gLuaManager().GetLuaState());

      glm::vec4 color = GetColor();
      luaObject.SetNumber("color_r", color.r);
      luaObject.SetNumber("color_g", color.g);
      luaObject.SetNumber("color_b", color.b);

      glm::vec3 dir = GetDirection();
      luaObject.SetNumber("dir_x", dir.x);
      luaObject.SetNumber("dir_y", dir.y);
      luaObject.SetNumber("dir_z", dir.z);

      glm::vec3 att = GetAtt();
      luaObject.SetNumber("att_x", att.x);
      luaObject.SetNumber("att_y", att.y);
      luaObject.SetNumber("att_z", att.z);

      glm::vec3 intensity = GetIntensity();
      luaObject.SetNumber("intensity_x", intensity.x);
      luaObject.SetNumber("intensity_y", intensity.y);
      luaObject.SetNumber("intensity_z", intensity.z);

      luaObject.SetInteger("type", GetLightType());
      luaObject.SetNumber("range", GetRange());
      luaObject.SetNumber("spot", GetSpot());

      return luaObject;
   }

   void CLight::SetColor(const glm::vec4& color)
   {
      mInternal->SetColor(color);
   }

   void CLight::SetDirection(const glm::vec3& direction)
   {
      mInternal->SetDirection(direction);
   }

   void CLight::SetDirection(float x, float y, float z)
   {
      mInternal->SetDirection(glm::vec3(x, y, z));
   }

   void CLight::SetRange(float range)
   {
      mInternal->SetRange(range);
   }

   void CLight::SetSpot(float spot)
   {
      mInternal->SetSpot(spot);
   }

   void CLight::SetAtt(float a0, float a1, float a2)
   {
      mInternal->SetAtt(a0, a1, a2);
   }

   void CLight::SetAttenuation(glm::vec3 attenuation)
   {
      mInternal->SetAtt(attenuation.x, attenuation.y, attenuation.z);
   }

   void CLight::SetType(Utopian::LightType type)
   {
      mInternal->SetType(type);
   }

   void CLight::SetIntensity(float ambient, float diffuse, float specular)
   {
      mInternal->SetIntensity(ambient, diffuse, specular);
   }

   void CLight::SetIntensity(glm::vec3 intensity)
   {
      SetIntensity(intensity.x, intensity.y, intensity.z);
   }

   const Utopian::LightData& CLight::GetLightData() const
   {
      return mInternal->GetLightData();
   }

   const glm::vec3& CLight::GetDirection() const
   {
      return mInternal->GetDirection();
   }

   const glm::vec3& CLight::GetAtt() const
   {
      return mInternal->GetAtt();
   }

   const glm::vec3& CLight::GetIntensity() const
   {
      return mInternal->GetIntensity();
   }

   const glm::vec4& CLight::GetColor() const
   {
      return mInternal->GetColor();
   }

   float CLight::GetRange() const
   {
      return mInternal->GetRange();
   }

   float CLight::GetSpot() const
   {
      return mInternal->GetSpot();
   }

   int CLight::GetLightType() const
   {
      return mInternal->GetType();
   }
}