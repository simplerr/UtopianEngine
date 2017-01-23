#pragma once

namespace ECS
{
	class Entity;

	class System
	{
	public:
		// The derived systems can store the entities however they want
		// RenderSystem groups them by their Pipeline 
		virtual void AddEntity(Entity* entity) =  0;
	protected:
		// Only the entities that have components inside this ECS::System
		//EntityList mEntities;
	};
}
