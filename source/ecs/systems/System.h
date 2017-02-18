#pragma once

namespace ECS
{
	class Entity;
	class EntityManager;

	class System
	{
	public:
		System(EntityManager* entityManager, uint32_t componentMask)
		{
			mEntityManager = entityManager;
			mComponentMask = componentMask;
		}

		EntityManager* GetEntityManager()
		{
			return mEntityManager;
		}

		bool Accepts(uint32_t mask)
		{
			return (mask & GetComponentMask()) == GetComponentMask();
		}

		uint32_t GetComponentMask()
		{
			return mComponentMask;
		}

		// The derived systems can store the entities however they want
		// RenderSystem groups them by their Pipeline 
		virtual void AddEntity(Entity* entity) =  0;
		virtual void RemoveEntity(Entity* entity) = 0;
		virtual void Process() = 0;
		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
		virtual bool Contains(Entity* entity) = 0;

	protected:
		// Only the entities that have components inside this ECS::System
		//EntityList mEntities;
		EntityManager* mEntityManager;
		uint32_t mComponentMask;
	};
}
