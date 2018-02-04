#include "Actor.h"
#include "scene/ObjectManager.h"
#include "scene/CTransform.h"
#include "ecs/SystemManager.h"
#include "Exception.h"

namespace Scene
{
	Actor::Actor(string name)
		: Object(name)
	{

	}

	SharedPtr<Actor> Actor::Create(string name)
	{
		SharedPtr<Actor> entity(new Actor(name));

		ObjectManager::Instance().RegisterObject(entity);
		World::Instance().NotifyEntityCreated(entity.get());

		// Let SceneManager assign root node

		return entity;
	}

	Vulkan::BoundingBox Actor::GetBoundingBox() const
	{
		Vulkan::BoundingBox boundingBox;
		//boundingBox.Init(GetTransform().GetPosition(), glm::vec3(5000.0f));
		boundingBox.Init(glm::vec3(0.0f), glm::vec3(0.0f));

		for (auto& component : mComponents)
		{
			Vulkan::BoundingBox box = component->GetBoundingBox();
			if (box.GetMin() == glm::vec3(0.0f) && box.GetMax() == glm::vec3(0.0f))
				continue;
			else
				return box;
		}

		return boundingBox;
	}

	const Transform& Actor::GetTransform() const
	{
		CTransform* transform = GetComponent<CTransform>();

		if (transform == nullptr)
			THROW_EXCEPTION(Exception, "No CTransform component");

		return transform->GetTransform();
	}
}