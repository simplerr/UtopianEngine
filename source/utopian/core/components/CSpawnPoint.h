#pragma once
#include "core/components/Component.h"
#include "utility/Common.h"

namespace Utopian
{
   class Actor;
   class CTransform;

   class CSpawnPoint : public Component
   {
   public:
      CSpawnPoint(Actor* parent);
      ~CSpawnPoint();

      void Update() override;
      void OnCreated() override;
      void OnDestroyed() override;
      void PostInit() override;

      LuaPlus::LuaObject GetLuaObject() override;

      // Type identification
      static uint32_t GetStaticType() {
         return Component::ComponentType::SPAWN_POINT;
      }

      virtual uint32_t GetType() {
         return GetStaticType();
      }

   private:
      CTransform* mTransform;
   };
}
