#pragma once
#include "scene/Object.h"
#include "Collision.h"

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
			FREE_CAMERA,
			ORBIT,
			PLAYER_CONTROL
		};

		SceneComponent(SceneEntity* parent);
		virtual ~SceneComponent();

		virtual void OnCreated() { };
		virtual void Update() { };

		const virtual Vulkan::BoundingBox GetBoundingBox() const;

		virtual uint32_t GetType() = 0;

		SceneEntity* GetParent() { return mParent; }

		void SetActive(bool active) { mActive = active; }
		void Activate() { mActive = true; }
		void Deactivate() { mActive = false; }
		bool IsActive() const { return mActive; }

	private:
		SceneEntity* mParent;
		bool mActive;
	};
}
