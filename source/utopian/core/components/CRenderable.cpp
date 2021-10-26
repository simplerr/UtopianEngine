#include "core/components/Actor.h"
#include "core/components/CRenderable.h"
#include "core/components/CTransform.h"
#include "core/renderer/Renderable.h"
#include "core/renderer/Model.h"
#include "core/World.h"
#include "core/ModelLoader.h"
#include "utility/Timer.h"
#include "im3d/im3d.h"

namespace Utopian
{
   CRenderable::CRenderable(Actor* parent)
      : Component(parent)
   {
      SetName("CRenderable");
   }

   CRenderable::~CRenderable()
   {

   }

   void CRenderable::Update()
   {
      mInternal->UpdateAnimation(gTimer().GetFrameTime());

      if (HasRenderFlags(RENDER_FLAG_BOUNDING_BOX))
      {
         glm::vec3 position = mInternal->GetTransform().GetPosition();
         BoundingBox aabb = GetBoundingBox();

         Im3d::PushEnableDepthTesting();
         Im3d::DrawAlignedBox(aabb.GetMin(), aabb.GetMax());
         Im3d::DrawPoint(position, 10.0f, Im3d::Color_Green);
         Im3d::PopEnableDepthTesting();
      }
   }

   void CRenderable::OnCreated()
   {
      mInternal = Renderable::Create();

      //mInternal = Renderer::Instance().CreateRenderable();
      World::Instance().BindNode(mInternal, GetParent());
   }

   void CRenderable::OnDestroyed()
   {
      mInternal->OnDestroyed();
      World::Instance().RemoveNode(mInternal);
   }

   void CRenderable::PostInit()
   {

   }

   LuaPlus::LuaObject CRenderable::GetLuaObject()
   {
      LuaPlus::LuaObject luaObject;
      luaObject.AssignNewTable(gLuaManager().GetLuaState());

      luaObject.SetString("path", GetPath().c_str());
      luaObject.SetNumber("render_flags", (lua_Number)GetRenderFlags());

      glm::vec4 color = GetColor();
      luaObject.SetNumber("color_r", color.r);
      luaObject.SetNumber("color_g", color.g);
      luaObject.SetNumber("color_b", color.b);
      luaObject.SetNumber("color_a", color.a);

      return luaObject;
   }

   void CRenderable::LoadModel(std::string path)
   {
      mPath = path;
      mInternal->LoadModel(path);
   }

   void CRenderable::SetModel(SharedPtr<Model> model)
   {
      // Note: Todo: How should ActorFactory that loads from Lua handle this?
      mPath = "Unknown";
      mInternal->SetModel(model);
   }

   Model* CRenderable::GetModel()
   {
      return mInternal->GetModel();
   }

   void CRenderable::SetDiffuseTexture(uint32_t materialIdx, SharedPtr<Vk::Texture> texture)
   {
      mInternal->SetDiffuseTexture(materialIdx, texture);
   }

   void CRenderable::SetNormalTexture(uint32_t materialIdx, SharedPtr<Vk::Texture> texture)
   {
      mInternal->SetNormalTexture(materialIdx, texture);
   }

   void CRenderable::SetSpecularTexture(uint32_t materialIdx, SharedPtr<Vk::Texture> texture)
   {
      mInternal->SetSpecularTexture(materialIdx, texture);
   }

   void CRenderable::SetTileFactor(glm::vec2 tileFactor)
   {
      mInternal->SetTileFactor(tileFactor);
   }

   void CRenderable::SetColor(glm::vec4 color)
   {
      mInternal->SetColor(color);
   }

   void CRenderable::SetPushFoliage(bool push)
   {
      mInternal->SetPushFoliage(push);
   }

   void CRenderable::SetVisible(bool visible)
   {
      mInternal->SetVisible(visible);
   }

   void CRenderable::SetRenderFlags(uint32_t renderFlags)
   {
      mInternal->SetRenderFlags(renderFlags);
   }

   void CRenderable::AppendRenderFlags(uint32_t renderFlags)
   {
      mInternal->AppendRenderFlags(renderFlags);
   }

   void CRenderable::RemoveRenderFlags(uint32_t renderFlags)
   {
      mInternal->RemoveRenderFlags(renderFlags);
   }

   void CRenderable::EnableBoundingBox()
   {
      mInternal->SetDrawBoundingBox(true);
   }

   void CRenderable::DisableBoundingBox()
   {
      mInternal->SetDrawBoundingBox(false);
   }

   const BoundingBox CRenderable::GetBoundingBox() const
   {
      return mInternal->GetBoundingBox();
   }

   const std::string CRenderable::GetPath() const
   {
      return mPath;
   }
   
   uint32_t CRenderable::GetRenderFlags() const
   {
      return mInternal->GetRenderFlags();
   }

   glm::vec2 CRenderable::GetTextureTiling() const
   {
      return mInternal->GetTextureTiling();
   }

   const bool CRenderable::HasRenderFlags(uint32_t renderFlags) const
   {
      return mInternal->HasRenderFlags(renderFlags);
   }

   const bool CRenderable::IsPushingFoliage() const
   {
      return mInternal->IsPushingFoliage();
   }

   const bool CRenderable::IsVisible() const
   {
      return mInternal->IsVisible();
   }

   glm::vec4 CRenderable::GetColor() const
   {
      return mInternal->GetColor();
   }

   SharedPtr<Renderable> CRenderable::GetInternal()
   {
      return mInternal;
   }
}