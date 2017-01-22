#pragma once
//#include "EntityManager.h"

namespace ECS
{
	class Entity;

	class System
	{
	public:
		//virtual void Update(Entity* entity) = 0;

		/*void AddEntity(Entity* entity) {
			mEntities.push_back(entity);
		}*/

	protected:
		// Only the entities that have components inside this ECS::System
		//EntityList mEntities;
	};
}
