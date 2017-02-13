#include "HealthComponent.h"

namespace ECS
{
	HealthComponent::HealthComponent(uint32_t health)
		: Component(ECS::Type::HEALTH_COMPONENT)
	{
		SetHealth(health);
	}

	void HealthComponent::SetHealth(uint32_t health)
	{
		mHealth = health;
	}

	void HealthComponent::AddHealth(uint32_t health)
	{
		mHealth += health;
	}

	uint32_t HealthComponent::GetHealth()
	{
		return mHealth;
	}
}