#include "vulkan/VulkanApp.h"
#include "systems/RenderSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/PickingSystem.h"
#include "components/Component.h"
#include "EntityManager.h"
#include "Entity.h"

namespace ECS
{
	EntityManager::EntityManager(VulkanLib::VulkanApp* vulkanApp)
	{
		// Create all ECS::System
		mPhysicsSystem = new ECS::PhysicsSystem();
		mRenderSystem = new ECS::RenderSystem(vulkanApp);
		mPickingSystem = new ECS::PickingSystem(vulkanApp->GetCamera(), vulkanApp);
		vulkanApp->SetRenderSystem(mRenderSystem);

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
		delete mRenderSystem;
		delete mPhysicsSystem;
		delete mPickingSystem;
	}

	// AddEntity() is responsible for informing the needed ECS::Systems
	Entity* EntityManager::AddEntity(ComponentList& components)
	{
		Entity* entity = new Entity(components, mNextEntityId);

		for (int i = 0; i < components.size(); i++)
		{
			// Add to RenderSystem
			if (components[i]->GetType() == MESH_COMPONENT)
			{
				// The RenderSystem will need to access the TransformComponent
				mRenderSystem->AddEntity(entity);
				mPickingSystem->AddEntity(entity);
			}
			else if (components[i]->GetType() == PHYSICS_COMPONENT)
			{
				mPhysicsSystem->AddEntity(entity);
			}
			else if (components[i]->GetType() == LIGHT_COMPONENT)
			{
				// Todo (should be in RenderSystem?)
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

	void EntityManager::Process()
	{
		mRenderSystem->Process();
		mPhysicsSystem->Process();
		mPickingSystem->Process();
	}
}