#include "Entity.h"
#include "Component.h"

namespace ECS
{
	Entity::Entity(ComponentList components, uint32_t id)
	{
		for (int i = 0; i < components.size(); i++)
		{
			mComponents.push_back(components[i]);
		}

		mId = id;
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

	//TransformComponent* Entity::GetTransform()
	//{
	//	return mTransformComponent;
	//}
}