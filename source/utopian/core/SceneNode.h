#pragma once
#include "core/Transform.h"

namespace Utopian
{
   /*
      Represents a node in the scene graph.
      Inherited by Light, Renderable, ParticleSystem etc.
   */
   class SceneNode
   {
   public:
      SceneNode();
      ~SceneNode();
      
      // Setters
      void SetTransform(const Transform& transform);
      void SetPosition(const glm::vec3& position);
      void SetRotation(const glm::vec3& rotation);
      void SetScale(const glm::vec3& scale);
      void SetId(uint32_t id);

      void AddTranslation(const glm::vec3& translation);
      void AddRotation(const glm::vec3& rotation);
      void AddScale(const glm::vec3& scale);

      void SetDrawBoundingBox(bool draw);

      // Getters
      const Transform& GetTransform() const;
      const glm::vec3& GetPosition() const;
      const glm::vec3& GetScale() const;
      const glm::mat4& GetWorldMatrix() const;
      uint32_t GetId() const;

      bool IsBoundingBoxVisible() const;
   private:
      uint32_t mId;
      Transform mTransform;
      bool mDrawBoundingBox;
   };
}