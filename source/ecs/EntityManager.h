#pragma once

#include <Window.h>
#include <vector>
#include "ecs/components/Component.h"

namespace Vulkan
{
	class Renderer;
	class Camera;
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
		EntityManager();
		~EntityManager();

		void Init(Vulkan::Renderer* renderer, Vulkan::Camera* camera);

		void AddSystem(ECS::System* system);
		Entity* AddEntity(ComponentList& components);
		Entity* GetEntity(uint32_t id);

		void RemoveEntity(Entity* entity);

		// It might be good to do this after all systems have been processed
		// To better support future multi-threading of systems
		void RemoveComponent(Entity* entity, ECS::Type componentType);
		void AddComponent(Entity* entity, Component* component);

		void Process();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		// All the ECS::System 
		std::vector<ECS::System*> mSystems;

		EntityList mEntities;
		EntityList mRemoveList;
		uint32_t mNextEntityId;
	};
}
