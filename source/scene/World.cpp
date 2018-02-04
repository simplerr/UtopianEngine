#include "scene/World.h"
#include "scene/Actor.h"
#include "scene/CRenderable.h"
#include "Collision.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Scene
{
	World::World()
	{

	}

	World::~World()
	{

	}

	void World::NotifyComponentCreated(SceneComponent* component)
	{
		component->OnCreated();

		mActiveComponents.push_back(component);
	}

	Actor* World::RayIntersection(const Vulkan::Ray& ray)
	{
		Actor* selectedEntity = nullptr;

		float minDistance = FLT_MAX;
		for (auto& entity : mEntities)
		{
			if (entity->HasComponent<CRenderable>())
			{
				Vulkan::BoundingBox boundingBox = entity->GetBoundingBox();

				float distance = FLT_MAX;
				if (boundingBox.RayIntersect(ray, distance))// && distance < minDistance)
				{
					selectedEntity = entity;
				}
			}
		}

		return selectedEntity;
	}

	void World::BindNode(const SharedPtr<SceneNode>& node, Actor* entity)
	{
		BoundNode binding;
		binding.node = node;
		binding.entity = entity;
		mBoundNodes[node.get()] = binding;
	}

	void World::Update()
	{
		// Synchronize transform between nodes and entities
		for (auto& entry : mBoundNodes)
		{
			entry.second.node->SetTransform(entry.second.entity->GetTransform());
		}
		
		// Update every active component
		for (auto& entry : mActiveComponents)
		{
			if (entry->IsActive())
			{
				entry->Update();
			}
		}
	}

	void World::NotifyEntityCreated(Actor * entity)
	{
		mEntities.push_back(entity);
	}
}