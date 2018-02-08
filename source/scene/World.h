#pragma once
#include <vector>
#include <map>
#include "scene/SceneComponent.h"
#include "scene/SceneNode.h"
#include "utility/Module.h"
#include "utility/Common.h"
#include "Collision.h"

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

		void NotifyComponentCreated(SceneComponent* component);

		Actor* RayIntersection(const Utopian::Vk::Ray& ray);

		/* The bound SceneNodes transform will be synchronized with the SceneEntity in Update() */
		void BindNode(const SharedPtr<SceneNode>& node, Actor* entity);
	private:
		vector<Actor*> mEntities;
		vector<SceneComponent*> mActiveComponents;
		map<SceneNode*, BoundNode> mBoundNodes;
	};
}
