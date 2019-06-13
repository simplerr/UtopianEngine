#include "Actor.h"
#include "core/ObjectManager.h"
#include "core/components/CTransform.h"
#include "utility/Exception.h"

namespace Utopian
{
	Actor::Actor(std::string name)
		: Object(name)
	{
		SetAlive(true);
	}

	Actor::~Actor()
	{
		volatile int test = 0;
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

		if (transform == nullptr)
			THROW_EXCEPTION(Exception, "No CTransform component");

		return transform->GetTransform();
	}

	std::vector<Component*>& Actor::GetComponents()
	{
		return mComponents;
	}
}