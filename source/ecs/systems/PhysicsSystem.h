#pragma once

#include <vector>
#include "System.h"

namespace VulkanLib
{
}

namespace ECS
{
	class TransformComponent;
	class PhysicsComponent;


	class PhysicsSystem : public System
	{
		struct EntityCache
		{
			Entity* entity;
			TransformComponent* transform;
			PhysicsComponent* physics;
		};

	public:
		void AddEntity(Entity* entity);
		void Process();
	private:
		std::vector<EntityCache> mEntities;
	};
}