#pragma once
#include "core/Object.h"
#include "utility/math/BoundingBox.h"

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
			PLAYER_CONTROL
		};

		Component(Actor* parent);
		virtual ~Component();

		virtual void OnCreated() { };
		virtual void Update() { };

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