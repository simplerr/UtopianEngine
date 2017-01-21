#pragma once

namespace ECS
{
	enum Type
	{
		TRANSFORM_COMPONENT,
		MESH_COMPONENT,
		PHYSICS_COMPONENT
	};

	class Component
	{

	public:
		Component(Component::Type type) {
			mType = type;
		}

		Type GetType() {
			return mType;
		}

		//virtual void Update(float dt) = 0;
		virtual void Draw() {};

	private:
		Type mType;
	};
}
