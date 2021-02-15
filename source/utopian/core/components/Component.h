#pragma once
#include "core/Object.h"
#include "utility/math/BoundingBox.h"
#include <LuaPlus.h>

namespace Utopian
{
	class Actor;

	class Component : public Object
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
			PLAYER_CONTROL,
			RANDOM_PATHS,
			BLOOM_LIGHT,
			RIGID_BODY,
			CATMULL_SPLINE,
			POLYMESH,
			SPAWN_POINT
		};

		Component(Actor* parent);
		virtual ~Component();

		virtual void OnCreated() { };
		virtual void OnDestroyed() { };
		virtual void Update() { };
		virtual void PostInit() = 0;

		virtual LuaPlus::LuaObject GetLuaObject() { return LuaPlus::LuaObject(); };
		virtual void SerializeData() { };

		const virtual Utopian::BoundingBox GetBoundingBox() const;

		virtual uint32_t GetType() = 0;

		Actor* GetParent() { return mParent; }

		void SetActive(bool active) { mActive = active; }
		void Activate() { mActive = true; }
		void Deactivate() { mActive = false; }
		bool IsActive() const { return mActive; }

	private:
		Actor* mParent;
		bool mActive;
	};
}
