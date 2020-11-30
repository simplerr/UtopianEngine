#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <vulkan/Mesh.h>
#include "core/World.h"

namespace Utopian
{
   class World;
   class Actor;

   class PrototypeTool
   {
      public:
         PrototypeTool();
         ~PrototypeTool();

         void Update(World* world, Actor* selectedActor);
         void PreFrame();
         void RenderUi();

         void ActorSelected(Actor* actor, glm::vec3 normal);
      private:
         Actor* AddBox(glm::vec3 position, std::string texture);

      private:
         std::vector<Actor*> mNewlyAddedActors;
         Actor* mLastSelectedActor = nullptr;
         glm::vec3 mLastAddedPosition = glm::vec3(FLT_MAX);

         const SceneLayer PrototypeBoxSceneLayer = 1u;

         Utopian::Vk::StaticModel* mSelectedModel = nullptr;
         glm::vec3 mSelectedFaceNormal;
         bool mRebuildMeshBuffer = false;
   };
}