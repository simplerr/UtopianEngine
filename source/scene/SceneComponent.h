#pragma once
#include "scene/Object.h"

namespace Scene
{
	class SceneEntity;

	class SceneComponent : public Object
	{
	public:

		enum ComponentType
		{
			TRANSFORM = 0,
			STATIC_MESH,
			LIGHT,
			CAMERA,
			FREE_CAMERA
		};

		SceneComponent(SceneEntity* parent);
		virtual ~SceneComponent();

		virtual void OnCreated() { };
		virtual void Update() { };

		virtual uint32_t GetType() = 0;

		SceneEntity* GetParent() { return mParent; }

	private:
		SceneEntity* mParent;
	};
}
