#pragma once

#include <Window.h>
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
	class PhysicsSystem;
	class PickingSystem;
	class HealthSystem;

	typedef std::vector<Entity*> EntityList;
	typedef std::vector<Component*> ComponentList;

	class EntityManager
	{
	public:
		EntityManager(VulkanLib::VulkanApp* vulkanApp);
		~EntityManager();

		void Init(VulkanLib::VulkanApp* vulkanApp);

		Entity* AddEntity(ComponentList& components);
		Entity* GetEntity(uint32_t id);

		void RemoveEntity(Entity* entity);

		void Process();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		// All the ECS::System 
		ECS::RenderSystem* mRenderSystem;
		ECS::PhysicsSystem* mPhysicsSystem;
		ECS::PickingSystem* mPickingSystem;
		ECS::HealthSystem* mHealthSystem;

		EntityList mEntities;
		EntityList mRemoveList;
		uint32_t mNextEntityId;
	};
}
