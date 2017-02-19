#include "System.h"
#include "ecs/Entity.h"
#include "ecs/components/TransformComponent.h"
#include "ecs/components/MeshComponent.h"
#include "ecs/components/HealthComponent.h"
#include "ecs/components/PhysicsComponent.h"

namespace ECS
{
	System::System(EntityManager* entityManager, uint32_t componentMask)
	{
		mEntityManager = entityManager;
		mComponentMask = componentMask;
	}

	EntityManager* System::GetEntityManager()
	{
		return mEntityManager;
	}

	bool System::Accepts(uint32_t mask)
	{
		return (mask & GetComponentMask()) == GetComponentMask();
	}

	void System::AddEntity(Entity* entity)
	{
		EntityCache entityCache;
		entityCache.entity = entity;
		entityCache.transformComponent = dynamic_cast<TransformComponent*>(entity->GetComponent(TRANSFORM_COMPONENT));
		entityCache.meshComponent = dynamic_cast<MeshComponent*>(entity->GetComponent(MESH_COMPONENT));
		entityCache.healthComponent = dynamic_cast<HealthComponent*>(entity->GetComponent(HEALTH_COMPONENT));
		entityCache.physicsComponent = dynamic_cast<PhysicsComponent*>(entity->GetComponent(PHYSICS_COMPONENT));

		mEntities.push_back(entityCache);

		OnEntityAdded(entityCache);
	}

	void System::RemoveEntity(Entity* entity)
	{
		for (auto iter = mEntities.begin(); iter < mEntities.end(); iter++)
		{
			if ((*iter).entity->GetId() == entity->GetId())
			{
				iter = mEntities.erase(iter);
				break;
			}
		}
	}

	bool System::Contains(Entity* entity)
	{
		for (EntityCache entityCache : mEntities)
		{
			if (entityCache.entity->GetId() == entity->GetId())
				return true;
		}

		return false;
	}

	uint32_t System::GetComponentMask()
	{
		return mComponentMask;
	}
}