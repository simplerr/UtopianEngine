#include "EntityManager.h"
#include "Entity.h"
#include "RenderSystem.h"
#include "Component.h"
#include "VulkanApp.h"

namespace ECS
{
	EntityManager::EntityManager(VulkanLib::VulkanApp* vulkanApp)
	{
		// Create all ECS::System
		mRenderSystem = new ECS::RenderSystem(vulkanApp);
		//vulkanApp->SetRenderSystem(mRenderSystem);
	}

	EntityManager::~EntityManager()
	{
		// Delete all entities
		for(int i = 0; i < mEntities.size(); i++)
		{
			delete mEntities[i];
		}

		// Delete all ECS::System
		delete mRenderSystem;
	}

	// AddEntity() is responsible for informing the needed ECS::Systems
	Entity* EntityManager::AddEntity(ComponentList& components)
	{
		Entity* entity = new Entity(components);

		for (int i = 0; i < components.size(); i++)
		{
			// Add to RenderSystem
			if (components[i]->GetType() == MESH_COMPONENT)
			{
				// The RenderSystem will need to access the TransformComponent
				mRenderSystem->AddMeshEntity(entity);
			}
			else if (components[i]->GetType() == PHYSICS_COMPONENT)
			{
				// Todo
			}
			else if (components[i]->GetType() == LIGHT_COMPONENT)
			{
				// Todo (should be in RenderSystem?)
			}
		}

		mEntities.push_back(entity);

		return entity;
	}

	Entity* EntityManager::GetEntity(uint32_t id)
	{
		return nullptr;
	}

	void EntityManager::Process()
	{
		mRenderSystem->Process();
	}
}