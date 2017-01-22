#pragma once
#include <vector>

namespace VulkanLib
{
	class VulkanApp;
}

namespace ECS
{
	class Entity;
	class Component;
	class RenderSystem;

	typedef std::vector<Entity*> EntityList;
	typedef std::vector<Component*> ComponentList;

	class EntityManager
	{
	public:
		EntityManager(VulkanLib::VulkanApp* vulkanApp);
		~EntityManager();
		Entity* AddEntity(ComponentList& components);
		Entity* GetEntity(uint32_t id);

		void Process();
	private:
		// All the ECS::System 
		ECS::RenderSystem* mRenderSystem;

		EntityList mEntities;
	};
}
