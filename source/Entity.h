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

	typedef std::vector<Component> ComponentList;

	class Entity
	{

	public:
		void AddComponent(Component* component);
		uint32_t GetId();
	private:
		ComponentList mComponents;
		uint32_t mId;

		// An entity should be able to have multiple MeshComponents
	};
}
