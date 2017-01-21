#pragma once

namespace ECS
{
	class Entity;

	class System
	{
	public:
		virtual void Update(Entity* entity) = 0;
	private:
	};
}
