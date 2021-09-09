#pragma once
#include "core/components/Component.h"
#include "core/renderer/Renderable.h"
#include "utility/Common.h"
#include "core/LuaManager.h"

namespace Utopian
{
   class Actor;
   class Model;

   class CRenderable : public Component
   {
   public:
      CRenderable(Actor* parent);
      virtual ~CRenderable();

      void Update() override;
      void OnCreated() override;
      void OnDestroyed() override;
      void PostInit() override;

      LuaPlus::LuaObject GetLuaObject() override;

      void LoadModel(std::string path);
      void SetModel(SharedPtr<Model> model);

      /**
       * @brief Set the texture to use when drawing
       *
       * @note Assmues the Model only consists of one Mesh
       */
      void SetTexture(SharedPtr<Vk::Texture> texture);
      void SetSpecularTexture(SharedPtr<Vk::Texture> texture);
      void SetTileFactor(glm::vec2 tileFactor);
      void SetColor(glm::vec4 color);
      void SetPushFoliage(bool push);
      void SetVisible(bool visible);

      // Render flag settings
      void SetRenderFlags(uint32_t renderFlags);
      void AppendRenderFlags(uint32_t renderFlags);
      void RemoveRenderFlags(uint32_t renderFlags);
      uint32_t GetRenderFlags() const;
      const bool HasRenderFlags(uint32_t renderFlags) const;
      const bool IsPushingFoliage() const;
      const bool IsVisible() const;

      // Todo: Remove
      void EnableBoundingBox();
      void DisableBoundingBox();

      // Type identification
      static uint32_t GetStaticType() {
         return Component::ComponentType::STATIC_MESH;
      }

      virtual uint32_t GetType() {
         return GetStaticType();
      }

      const BoundingBox GetBoundingBox() const;
      const std::string GetPath() const;
      glm::vec2 GetTextureTiling() const;
      glm::vec4 GetColor() const;

      SharedPtr<Renderable> GetInternal();

   private:
      SharedPtr<Renderable> mInternal;
      std::string mPath;
   };
}
