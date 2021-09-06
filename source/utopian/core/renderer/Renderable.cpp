#include "core/renderer/Renderable.h"
#include "core/renderer/Renderer.h"
#include "vulkan/StaticModel.h"
#include <glm/gtc/matrix_transform.hpp>
#include "utility/math/BoundingBox.h"
#include "core/ModelLoader.h"

namespace Utopian
{
   Renderable::Renderable()
   {
      SetRenderFlags(RENDER_FLAG_DEFERRED | RENDER_FLAG_CAST_SHADOW);
      SetTileFactor(glm::vec2(1.0f, 1.0f));
      SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
      SetVisible(true);
      SetPushFoliage(false);
   }

   Renderable::~Renderable()
   {

   }

   SharedPtr<Renderable> Renderable::Create()
   {
      SharedPtr<Renderable> instance(new Renderable());
      instance->Initialize();

      return instance;
   }

   void Renderable::OnDestroyed()
   {
      Renderer::Instance().RemoveRenderable(this);
   }

   void Renderable::Initialize()
   {
      // Add new instance to the VulkanApp (scene)
      Renderer::Instance().AddRenderable(this);
   }

   Utopian::Vk::StaticModel* Renderable::GetModel()
   {
      return mModel.get();
   }

   void Renderable::LoadModel(std::string path)
   {
      mModel = Vk::gModelLoader().LoadModel(path);
   }

   void Renderable::SetModel(SharedPtr<Vk::StaticModel> model)
   {
      mModel = model;
   }

   void Renderable::SetTexture(SharedPtr<Vk::Texture> texture)
   {
      mModel->mMeshes[0]->SetTexture(texture);
   }

   void Renderable::SetSpecularTexture(SharedPtr<Vk::Texture> texture)
   {
      mModel->mMeshes[0]->SetTexture(texture);
   }

   void Renderable::SetTileFactor(glm::vec2 tileFactor)
   {
      mTextureTileFactor = tileFactor;
   }

   void Renderable::SetColor(glm::vec4 color)
   {
      mColor = color;
   }

   void Renderable::SetVisible(bool visible)
   {
      mVisible = visible;
   }

   void Renderable::SetPushFoliage(bool push)
   {
      mPushFoliage = push;
   }

   void Renderable::SetRenderFlags(uint32_t renderFlags)
   {
      mRenderFlags = renderFlags;
   }

   void Renderable::AppendRenderFlags(uint32_t renderFlags)
   {
      mRenderFlags |= renderFlags;
   }

   void Renderable::RemoveRenderFlags(uint32_t renderFlags)
   {
      mRenderFlags &= (~renderFlags);
   }

   const BoundingBox Renderable::GetBoundingBox() const
   {
      BoundingBox boundingBox = mModel->GetBoundingBox();
      boundingBox.Update(GetWorldMatrix());

      return boundingBox;
   }

   const glm::vec4 Renderable::GetColor() const
   {
      return mColor;
   }

   glm::vec2 Renderable::GetTextureTiling() const
   {
      return mTextureTileFactor;
   }

   const bool Renderable::IsVisible() const
   {
      return mVisible;
   }

   const bool Renderable::IsPushingFoliage() const
   {
      return mPushFoliage;
   }

   const uint32_t Renderable::GetRenderFlags() const
   {
      return mRenderFlags;
   }

   const bool Renderable::HasRenderFlags(uint32_t renderFlags) const
   {
      return (mRenderFlags & renderFlags) == renderFlags;
   }
}