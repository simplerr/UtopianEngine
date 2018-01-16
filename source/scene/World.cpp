#include "scene/World.h"
#include "scene/SceneEntity.h"

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


	void World::BindNode(const SharedPtr<SceneNode>& node, SceneEntity* entity)
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
}