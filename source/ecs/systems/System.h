#pragma once

namespace ECS
{
	class Entity;

	class System
	{
	public:
		System(uint32_t componentMask)
			: mComponentMask(componentMask) 
		{

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

	protected:
		// Only the entities that have components inside this ECS::System
		//EntityList mEntities;
		uint32_t mComponentMask;
	};
}
