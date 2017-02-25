#include "vulkan/VulkanApp.h"
#include "vulkan/VulkanDebug.h"
#include "systems/RenderSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/PickingSystem.h"
#include "systems/HealthSystem.h"
#include "components/Component.h"
#include "EntityManager.h"
#include "Entity.h"

namespace ECS
{
	EntityManager::EntityManager()
	{
		mNextEntityId = 0u;
	}

	EntityManager::~EntityManager()
	{
		// Delete all entities
		for(int i = 0; i < mEntities.size(); i++)
		{
			delete mEntities[i];
		}

		// Delete all ECS::System
		for(System* system : mSystems)
		{
			delete system;
		}
	}

	void EntityManager::Init(VulkanLib::VulkanApp* vulkanApp,  VulkanLib::Camera* camera)
	{
		// Create all ECS::System
		AddSystem(new ECS::PhysicsSystem(this));
		AddSystem(new ECS::PickingSystem(this, camera));
		AddSystem(new ECS::HealthSystem(this));
		AddSystem(new ECS::RenderSystem(this, vulkanApp));
	}

	void EntityManager::AddSystem(ECS::System* system)
	{
		mSystems.push_back(system);
	}

	// AddEntity() is responsible for informing the needed ECS::Systems
	Entity* EntityManager::AddEntity(ComponentList& components)
	{
		Entity* entity = new Entity(components, mNextEntityId);

		for (System* system : mSystems)
		{
			if (system->Accepts(entity->GetComponentsMask()))
			{
				system->AddEntity(entity);
			}
		}

		mEntities.push_back(entity);
		mNextEntityId++;

		return entity;
	}

	Entity* EntityManager::GetEntity(uint32_t id)
	{
		for (int i = 0; i < mEntities.size(); i++)
		{
			if (mEntities[i]->GetId() == id)
			{
				return mEntities[i];
			}
		}

		return nullptr;
	}

	void EntityManager::RemoveEntity(Entity* entity)
	{
		mRemoveList.push_back(entity);
	}

	void EntityManager::RemoveComponent(Entity* entity, ECS::Type componentType)
	{
		// Remove component from Entity and inform all related Systems
		if (entity->RemoveComponent(componentType))
		{
			for (System* system : mSystems)
			{
				if ((system->GetComponentMask() & componentType) != 0)
				{
					system->RemoveEntity(entity);
				}
			}
		}
	}

	void EntityManager::AddComponent(Entity* entity, Component* component)
	{
		entity->AddComponent(component);

		for (System* system : mSystems)
		{
			if (system->Accepts(entity->GetComponentsMask()))
			{
				if (system->Contains(entity) == false)
				{
					system->AddEntity(entity);
				}
			}
		}
	}

	void EntityManager::Process()
	{
		for (System* system : mSystems)
		{
			system->Process();
		}

		// Removing of entities is done after all systems have been processed
		for (auto entity : mRemoveList)
		{
			for (System* system : mSystems)
			{
				system->RemoveEntity(entity);
			}

			for (auto iter = mEntities.begin(); iter < mEntities.end(); iter++)
			{
				if ((*iter)->GetId() == entity->GetId())
				{
					iter = mEntities.erase(iter);
					break;
				}
			}
		}

		if (mRemoveList.size() != 0)
			mRemoveList.clear();
	}

	void EntityManager::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		for (System* system : mSystems)
		{
			system->HandleMessages(hWnd, uMsg, wParam, lParam);
		}

		switch (uMsg)
		{
		case WM_KEYDOWN:
			if (wParam == VK_SPACE)
			{
				VulkanLib::VulkanDebug::ConsolePrint("TODO: Add entity systems debug printing");
			}
			break;
		default:
			break;
		}
	}
}