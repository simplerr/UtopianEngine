#pragma once
#include <glm/glm.hpp>
#include "core/components/Component.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"

namespace Utopian
{
   class Actor;
   class CCamera;
   class CTransform;
   class CLight;
   class CRenderable;
   class Terrain;

   /*
   * Moves the Actor to randomized position on the terrain.
   */
   class CRandomPaths : public Component
   {
   public:
      CRandomPaths(Actor* parent, Terrain* terrain);
      ~CRandomPaths();

      void Update(double deltaTime) override;
      void OnCreated() override;
      void OnDestroyed() override;
      void PostInit() override;

      LuaPlus::LuaObject GetLuaObject() override;

      // Type identification
      static uint32_t GetStaticType() {
         return Component::ComponentType::BLOOM_LIGHT;
      }

      virtual uint32_t GetType() {
         return GetStaticType();
      }

   private:
      glm::vec2 GenerateNewTarget();
   private:
      CTransform* mTransform;
      Terrain* mTerrain;
      glm::vec2 mTarget;
      float mSpeed;
   };
}
