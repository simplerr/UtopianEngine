#include "core/World.h"
#include "core/components/Actor.h"
#include "core/components/CRenderable.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Utopian
{
	World::World()
	{

	}

	World::~World()
	{

	}

	void World::RemoveActor(Actor* actor)
	{
		// Removes the Actor and all of it's components
	}

	Actor* World::RayIntersection(const Ray& ray)
	{
		Actor* selectedActor = nullptr;

		float minDistance = FLT_MAX;
		for (auto& actor : mActors)
		{
			if (actor->HasComponent<CRenderable>())
			{
				BoundingBox boundingBox = actor->GetBoundingBox();

				float distance = FLT_MAX;
				if (boundingBox.RayIntersect(ray, distance))// && distance < minDistance)
				{
					selectedActor = actor.get();
				}
			}
		}

		return selectedActor;
	}

	vector<SharedPtr<Actor>>& World::GetActors()
	{
		return mActors;
	}

	void World::BindNode(const SharedPtr<SceneNode>& node, Actor* actor)
	{
		BoundNode binding;
		binding.node = node;
		binding.actor = actor;
		mBoundNodes[node.get()] = binding;
	}

	void World::RemoveNode(const SharedPtr<SceneNode>& node)
	{
		if (mBoundNodes.find(node.get()) != mBoundNodes.end())
		{
			mBoundNodes.erase(node.get());
		}
	}

	void World::Update()
	{
		// Loop through actors and check if any should be removed
		for (auto iter = mActors.begin(); iter != mActors.end();)
		{
			SharedPtr<Actor> actor = (*iter);
			if (!actor->IsAlive())
			{
				vector<Component*> components = actor->GetComponents();
				for (auto& component : components)
					component->OnDestroyed();

				iter = mActors.erase(iter);
			}
			else
			{
				iter++;
			}
		}

		// Synchronize transform between nodes and entities
		for (auto& entry : mBoundNodes)
		{
			entry.second.node->SetTransform(entry.second.actor->GetTransform());
		}
		
		// Update every active component
		for (auto& entry : mComponents)
		{
			if (entry->IsActive())
			{
				entry->Update();
			}
		}
	}

	void World::AddActor(const SharedPtr<Actor>& actor)
	{
		mActors.push_back(actor);
	}

	void World::AddComponent(const SharedPtr<Component>& component)
	{
		component->OnCreated();
		mComponents.push_back(component);
	}
}