#include <glm/gtc/matrix_transform.hpp>
#include "core/renderer/Renderable.h"
#include "core/renderer/Renderer.h"
#include "core/renderer/Model.h"
#include "core/ModelLoader.h"
#include "utility/math/BoundingBox.h"
#include "vulkan/handles/DescriptorSet.h"

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

   Model* Renderable::GetModel()
   {
      return mModel.get();
   }

   void Renderable::LoadModel(std::string path)
   {
      mModel = gModelLoader().LoadModel(path);
   }

   void Renderable::SetModel(SharedPtr<Model> model)
   {
      mModel = model;
   }

   void Renderable::SetDiffuseTexture(uint32_t materialIdx, SharedPtr<Vk::Texture> texture)
   {
      Material* material = mModel->GetMaterial(materialIdx);
      material->colorTexture = texture;
      material->BindTextureDescriptors(gRenderer().GetDevice());
   }

   void Renderable::SetNormalTexture(uint32_t materialIdx, SharedPtr<Vk::Texture> texture)
   {
      Material* material = mModel->GetMaterial(materialIdx);
      material->normalTexture = texture;
      material->BindTextureDescriptors(gRenderer().GetDevice());
   }

   void Renderable::SetSpecularTexture(uint32_t materialIdx, SharedPtr<Vk::Texture> texture)
   {
      Material* material = mModel->GetMaterial(materialIdx);
      material->specularTexture = texture;
      material->BindTextureDescriptors(gRenderer().GetDevice());
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