#include "ecs/components/HealthComponent.h"
#include "ecs/SystemManager.h"
#include "ecs/Entity.h"
#include "HealthSystem.h"

namespace ECS
{
	HealthSystem::HealthSystem(SystemManager* entityManager)
		: System(entityManager, Type::HEALTH_COMPONENT, SystemId::HEALTH_SYSTEM)
	{

	}

	void HealthSystem::Process()
	{
		for (auto& entityCache : mEntities)
		{
			// Remove entity from world
			if (entityCache.healthComponent->GetHealth() <= 0)
			{
				GetEntityManager()->RemoveEntity(entityCache.entity);
				//mEntityManager->RemoveComponent(entityCache.entity, ECS::Type::PHYSICS_COMPONENT);
			}
		}
	}

	void HealthSystem::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{

	}
}