#pragma once
#include <vector>

namespace ECS
{
	/*
		Some examples of different Components:

		MeshComponent, PhysicsComponent, AudioComponent, TransformComponent, NetworkComponent
		ParticleComponent, PowerupComponent, LightComponent, InputComponent, CameraComponent
	*/
	class Component;
	class TransformComponent;

	typedef std::vector<Component*> ComponentList;

	class Entity
	{
	public:
		Entity(ComponentList components, uint32_t id);
		Component* GetComponent(uint32_t componentType);
		uint32_t GetId();
		//TransformComponent* GetTransform();
	private:
		ComponentList mComponents;
		//TransformComponent* mTransformComponent;
		uint32_t mId;

		// An entity should be able to have multiple MeshComponents
	};
}
