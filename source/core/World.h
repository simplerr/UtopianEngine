#pragma once
#include <vector>
#include <map>
#include "core/components/Component.h"
#include "core/SceneNode.h"
#include "utility/Module.h"
#include "utility/Common.h"
#include "utility/math/Ray.h"

namespace Utopian
{
	class Actor;
	class SceneNode;

	struct BoundNode
	{
		SharedPtr<SceneNode> node;
		Actor* entity;
	};

	/*
		Manages all Entities and Components, also synchronizes the position of every
		SceneNode bound to SceneEntities.
	*/
	class World : public Module<World>
	{
	public:
		World();
		~World();
	
		void Update();
		void NotifyEntityCreated(Actor* entity);

		void NotifyComponentCreated(Component* component);

		Actor* RayIntersection(const Ray& ray);
		vector<Actor*>& GetActors();

		/* The bound SceneNodes transform will be synchronized with the SceneEntity in Update() */
		void BindNode(const SharedPtr<SceneNode>& node, Actor* entity);
	private:
		vector<Actor*> mEntities;
		vector<Component*> mActiveComponents;
		map<SceneNode*, BoundNode> mBoundNodes;
	};
}
