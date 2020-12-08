#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "core/World.h"

namespace Utopian
{
   class World;
   class Actor;
   class CPolyMesh;

   class PrototypeTool
   {
   public:
      PrototypeTool();
      ~PrototypeTool();

      void Update(World* world, Actor* selectedActor);
      void PreFrame();
      void RenderUi();

      void ActorSelected(Actor* actor);
    private:
      Actor* mSelectedActor = nullptr;
      CPolyMesh* mSelectedMesh = nullptr;
      bool mSelected = false;
   };
}