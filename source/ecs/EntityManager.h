#pragma once

#include <Window.h>
#include <vector>
#include "ecs/components/Component.h"

namespace VulkanLib
{
	class VulkanApp;
}

namespace ECS
{
	class System;
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

		void AddSystem(ECS::System* system);
		Entity* AddEntity(ComponentList& components);
		Entity* GetEntity(uint32_t id);

		void RemoveEntity(Entity* entity);
		void RemoveComponent(Entity* entity, ECS::Type componentType);
		void AddComponent(Entity* entity, Component* component);

		void Process();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		// All the ECS::System 
		std::vector<ECS::System*> mSystems;

		ECS::RenderSystem* mRenderSystem;
		ECS::PhysicsSystem* mPhysicsSystem;
		ECS::PickingSystem* mPickingSystem;
		ECS::HealthSystem* mHealthSystem;

		EntityList mEntities;
		EntityList mRemoveList;
		uint32_t mNextEntityId;
	};
}
