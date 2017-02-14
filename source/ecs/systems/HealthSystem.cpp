#include "ecs/components/HealthComponent.h"
#include "ecs/EntityManager.h"
#include "ecs/Entity.h"
#include "HealthSystem.h"

namespace ECS
{
	HealthSystem::HealthSystem(EntityManager* entityManager)
	{
		mEntityManager = entityManager;
	}

	void HealthSystem::Process()
	{
		for (auto& entityCache : mEntities)
		{
			// Remove entity from world
			if (entityCache.healthComponent->GetHealth() <= 0)
			{
				mEntityManager->RemoveEntity(entityCache.entity);
			}
		}
	}

	void HealthSystem::AddEntity(Entity* entity)
	{
		EntityCache entityCache;
		entityCache.entity = entity;
		entityCache.healthComponent = dynamic_cast<HealthComponent*>(entity->GetComponent(ECS::Type::HEALTH_COMPONENT));

		mEntities.push_back(entityCache);
	}

	void HealthSystem::RemoveEntity(Entity* entity)
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

	void HealthSystem::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{

	}
}