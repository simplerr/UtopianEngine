#include "EntityManager.h"
#include "RenderSystem.h"

namespace ECS
{
	EntityManager::EntityManager(VulkanLib::VulkanApp* vulkanApp)
	{
		// Create all ECS::System
		mRenderSystem = new ECS::RenderSystem(vulkanApp);
	}

	EntityManager::~EntityManager()
	{
		// Delete all ECS::System
		delete mRenderSystem;
	}

	// AddEntity() is responsible for informing the needed ECS::Systems
	Entity* EntityManager::AddEntity(const ComponentList& components)
	{
		return nullptr;
	}

	Entity* EntityManager::GetEntity(uint32_t id)
	{
		return nullptr;
	}
}