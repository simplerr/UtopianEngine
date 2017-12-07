#pragma once

#include <stdint.h>
#include <vector>
#include <windows.h>

namespace ECS
{
	class Entity;
	class SystemManager;
	class MeshComponent;
	class HealthComponent;
	class PhysicsComponent;
	class TransformComponent;

	struct EntityCache
	{
		EntityCache()
		{
			entity = nullptr;
			meshComponent = nullptr;
			transformComponent = nullptr;
			healthComponent = nullptr;
			physicsComponent = nullptr;
		}

		Entity* entity;
		MeshComponent* meshComponent;
		TransformComponent* transformComponent;
		HealthComponent* healthComponent;
		PhysicsComponent* physicsComponent;
	};

	class System
	{
	public:
		System(SystemManager* entityManager, uint32_t componentMask);
		virtual ~System();
		SystemManager* GetEntityManager();
		bool Accepts(uint32_t mask);
		uint32_t GetComponentMask();
		

		// The derived systems can store the entities however they want
		// RenderSystem groups them by their Pipeline 
		void AddEntity(Entity* entity);
		void RemoveEntity(Entity* entity);
		bool Contains(Entity* entity);

		virtual void OnEntityAdded(const EntityCache& entityCache) {};
		virtual void OnEntityRemoved(const EntityCache& entityCache) {};
		virtual void Process() = 0;
		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	protected:
		// Only the entities that have components inside this ECS::System
		//EntityList mEntities;
		SystemManager* mEntityManager;
		std::vector<EntityCache> mEntities;
		uint32_t mComponentMask;
	};
}
