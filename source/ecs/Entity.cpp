#include "components/Component.h"
#include "Entity.h"

namespace ECS
{
	Entity::Entity(ComponentList components, uint32_t id)
	{
		for (Component* component : components)
		{
			AddComponent(component);
		}

		mId = id;
	}

	void Entity::AddComponent(Component* component)
	{
		mComponents.push_back(component);
	}

	bool Entity::RemoveComponent(Type componentType)
	{
		for (auto iter = mComponents.begin(); iter != mComponents.end(); iter++)
		{
			if ((*iter)->GetType() == componentType)
			{
				mComponents.erase(iter);
				return true;
			}
		}

		return false;
	}

	Component* Entity::GetComponent(uint32_t componentType)
	{
		for (int i = 0; i < mComponents.size(); i++)
		{
			if (mComponents[i]->GetType() == componentType)
			{
				return mComponents[i];
			}
		}

		return nullptr;
	}

	uint32_t Entity::GetId()
	{
		return mId;
	}

	uint32_t Entity::GetComponentsMask()
	{
		uint32_t mask = 0;
		for (Component* component : mComponents)
		{
			mask |= (uint32_t)component->GetType();
		}

		return mask;
	}

	bool Entity::HasComponent(Type componentType)
	{
		for (Component* component : mComponents)
		{
			if (component->GetType() == componentType)
			{
				return true;
			}
		}

		return false;
	}

	//TransformComponent* Entity::GetTransform()
	//{
	//	return mTransformComponent;
	//}
}