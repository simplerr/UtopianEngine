#pragma once

namespace ECS
{
	enum Type
	{
		TRANSFORM_COMPONENT,
		MESH_COMPONENT,
		PHYSICS_COMPONENT,
		LIGHT_COMPONENT
	};

	class Component
	{

	public:
		Component(Component::Type type) {
			mType = type;
			mEntityId = 0u;
		}

		Type GetType() const {
			return mType;
		}

		uint32_t GetEntityId() const {
			return mEntityId;
		}

		//virtual void Update(float dt) = 0;
		virtual void Draw() {};

	private:
		Type mType;

		// The system for each component needs to know which entity the component is a part of
		// RenderSystem needs to query the TransformComponent
		uint32_t mEntityId;
	};
}
