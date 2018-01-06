#include "ecs/components/PhysicsComponent.h"
#include "ecs/components/TransformComponent.h"
#include "ecs/Entity.h"
#include "PhysicsSystem.h"

namespace ECS
{
	PhysicsSystem::PhysicsSystem()
		: System(Type::PHYSICS_COMPONENT, SystemId::PHYSICS_SYSTEM)
	{
	}

	void PhysicsSystem::Process()
	{
		for (int i = 0; i < mEntities.size(); i++)
		{
			TransformComponent* transform = mEntities[i].transformComponent;
			PhysicsComponent* physicsComponent = mEntities[i].physicsComponent;

			// Apply physics rules
			if (fabs(transform->GetPosition().x) > 1000.0f)
				physicsComponent->mVelocity.x *= -1;
			if (fabs(transform->GetPosition().y) > 1000.0f)
				physicsComponent->mVelocity.y *= -1;
			if (fabs(transform->GetPosition().z) > 1000.0f)
				physicsComponent->mVelocity.z *= -1;

			// Update the transform component
			transform->SetPosition(transform->GetPosition() + physicsComponent->GetVelocity());
			transform->SetRotation(transform->GetRotation() + physicsComponent->GetRotationSpeed());
			transform->SetScale(transform->GetScale() + physicsComponent->GetScaleSpeed());
		}
	}

	void PhysicsSystem::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{

	}
}