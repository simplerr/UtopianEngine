#include "vulkan/Renderer.h"
#include "vulkan/VulkanDebug.h"
#include "systems/RenderSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/EditorSystem.h"
#include "systems/HealthSystem.h"
#include "components/Component.h"
#include "SystemManager.h"
#include "Entity.h"

namespace ECS
{
	SystemManager::SystemManager()
	{
		mNextEntityId = 0u;
	}

	SystemManager::~SystemManager()
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

	void SystemManager::Init(Vulkan::Renderer* renderer, Vulkan::Camera* camera, Terrain* terrain, Input* input)
	{
		// Create all ECS::System
		AddSystem(new ECS::PhysicsSystem(this)); // terrain
		AddSystem(new ECS::EditorSystem(this, camera, terrain, input));
		AddSystem(new ECS::HealthSystem(this));
		AddSystem(new ECS::RenderSystem(this, renderer, camera, terrain));
	}

	void SystemManager::AddSystem(ECS::System* system)
	{
		mSystems.push_back(system);
	}

	// AddEntity() is responsible for informing the needed ECS::Systems
	Entity* SystemManager::AddEntity(ComponentList& components)
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

	Entity* SystemManager::GetEntity(uint32_t id)
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

	void SystemManager::RemoveEntity(Entity* entity)
	{
		mRemoveList.push_back(entity);
	}

	void SystemManager::RemoveComponent(Entity* entity, ECS::Type componentType)
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

	void SystemManager::AddComponent(Entity* entity, Component* component)
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

	void SystemManager::Process()
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

	void SystemManager::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
				Vulkan::VulkanDebug::ConsolePrint("TODO: Add entity systems debug printing");
			}
			break;
		default:
			break;
		}
	}
	
	ECS::System* SystemManager::GetSystem(SystemId id)
	{
		for (System* system : mSystems)
		{
			if (system->GetSystemId() == id)
				return system;
		}

		return nullptr;
	}
}