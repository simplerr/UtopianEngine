#pragma once
#include <vector>
#include "Entity.h"

namespace VulkanLib
{
	class VulkanApp;
}

namespace ECS
{
	class Entity;
	class RenderSystem;

	typedef std::vector<Entity*> EntityList;

	class EntityManager
	{
	public:
		EntityManager(VulkanLib::VulkanApp* vulkanApp);
		~EntityManager();
		Entity* AddEntity(const ComponentList& components);
		Entity* GetEntity(uint32_t id);
	private:
		// All the ECS::System 
		ECS::RenderSystem* mRenderSystem;

		EntityList mEntities;
	};
}
