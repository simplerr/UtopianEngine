#pragma once

#include "Component.h"

namespace ECS
{
	class HealthComponent : public Component
	{
	public:
		HealthComponent(uint32_t health);

		void SetHealth(uint32_t health);
		void AddHealth(uint32_t health);

		uint32_t GetHealth();
	private:
		uint32_t mHealth;
	};
}