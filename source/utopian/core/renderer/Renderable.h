#pragma once
#include <glm/glm.hpp>
#include "core/SceneNode.h"
#include "core/components/Component.h"
#include "utility/Common.h"
#include "vulkan/VulkanPrerequisites.h"

namespace Utopian
{
   class Actor;

   enum RenderFlags
   {
      RENDER_FLAG_DEFERRED = 1,
      RENDER_FLAG_COLOR = 2,
      RENDER_FLAG_NORMAL_DEBUG = 4,
      RENDER_FLAG_BOUNDING_BOX = 8,
      RENDER_FLAG_WIREFRAME = 16,
      RENDER_FLAG_CAST_SHADOW = 32,
      RENDER_FLAG_DRAW_OUTLINE = 64,
   };

   class Renderable : public SceneNode
   {
   public:
      Renderable();
      ~Renderable();

      void Initialize();
      void OnDestroyed();

      static SharedPtr<Renderable> Create();

      void LoadModel(std::string path);

      void SetModel(SharedPtr<Vk::StaticModel> model);
      void SetTexture(SharedPtr<Vk::Texture> texture);
      void SetSpecularTexture(SharedPtr<Vk::Texture> texture);
      void SetTileFactor(glm::vec2 tileFactor);
      void SetColor(glm::vec4 color);
      void SetVisible(bool visible);
      void SetPushFoliage(bool push);
      void SetRenderFlags(uint32_t renderFlags);
      void AppendRenderFlags(uint32_t renderFlags);
      void RemoveRenderFlags(uint32_t renderFlags);

      Utopian::Vk::StaticModel* GetModel();
      const BoundingBox GetBoundingBox() const;
      const glm::vec4 GetColor() const;
      const bool IsVisible() const;
      const bool IsPushingFoliage() const;
      const uint32_t GetRenderFlags() const;
      glm::vec2 GetTextureTiling() const;

      const bool HasRenderFlags(uint32_t renderFlags) const;

   private:
      SharedPtr<Utopian::Vk::StaticModel> mModel;
      glm::vec4 mColor;
      glm::vec2 mTextureTileFactor;
      uint32_t mRenderFlags;
      bool mVisible;
      bool mPushFoliage;
   };
}
