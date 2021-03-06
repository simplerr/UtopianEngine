#include "Actor.h"
#include "core/ObjectManager.h"
#include "core/components/CTransform.h"

namespace Utopian
{
   Actor::Actor(std::string name)
      : Object(name)
   {
      SetAlive(true);
      SetSerialize(true);
      SetSceneLayer(0u);
   }

   Actor::~Actor()
   {
   }

   SharedPtr<Actor> Actor::Create(std::string name)
   {
      SharedPtr<Actor> actor(new Actor(name));

      World::Instance().AddActor(actor);

      // Let SceneManager assign root node

      return actor;
   }

   void Actor::PostInit()
   {
      for (auto& component : mComponents)
      {
         component->PostInit();
      }
   }

   void Actor::SetAlive(bool alive)
   {
      mAlive = alive;
   }

   bool Actor::IsAlive() const
   {
      return mAlive;
   }

   void Actor::SetSerialize(bool serialize)
   {
      mSerialize = serialize;
   }

   bool Actor::ShouldSerialize() const
   {
      return mSerialize;
   }

   BoundingBox Actor::GetBoundingBox() const
   {
      BoundingBox boundingBox;
      //boundingBox.Init(GetTransform().GetPosition(), glm::vec3(5000.0f));
      boundingBox.Init(glm::vec3(0.0f), glm::vec3(0.0f));

      for (auto& component : mComponents)
      {
         BoundingBox box = component->GetBoundingBox();
         if (box.GetMin() == glm::vec3(0.0f) && box.GetMax() == glm::vec3(0.0f))
            continue;
         else
            return box;
      }

      return boundingBox;
   }

   Transform& Actor::GetTransform()
   {
      CTransform* transform = GetComponent<CTransform>();

      assert(transform);

      return transform->GetTransform();
   }

   std::vector<Component*>& Actor::GetComponents()
   {
      return mComponents;
   }

   void Actor::SetSceneLayer(SceneLayer sceneLayer)
   {
      mSceneLayer = sceneLayer;
   }

   SceneLayer Actor::GetSceneLayer() const
   {
      return mSceneLayer;
   }
}