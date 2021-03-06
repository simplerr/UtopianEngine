#pragma once
#include "core/components/Component.h"
#include "utility/Common.h"

namespace Utopian
{
   class Actor;
   class CTransform;
   class CRenderable;

   class CFinishPoint : public Component
   {
   public:
      CFinishPoint(Actor* parent);
      ~CFinishPoint();

      void Update() override;
      void OnCreated() override;
      void OnDestroyed() override;
      void PostInit() override;

      LuaPlus::LuaObject GetLuaObject() override;

      // Type identification
      static uint32_t GetStaticType() {
         return Component::ComponentType::FINISH_POINT;
      }

      virtual uint32_t GetType() {
         return GetStaticType();
      }

   private:
      CTransform* mTransform;
   };
}
