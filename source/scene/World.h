#pragma once
#include <vector>
#include <map>
#include "scene/SceneComponent.h"
#include "scene/SceneNode.h"
#include "utility/Module.h"
#include "utility/Common.h"
#include "Collision.h"

namespace Scene
{
	class SceneEntity;
	class SceneNode;

	struct BoundNode
	{
		SharedPtr<SceneNode> node;
		SceneEntity* entity;
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
		void NotifyEntityCreated(SceneEntity* entity);

		void NotifyComponentCreated(SceneComponent* component);

		SceneEntity* RayIntersection(const Vulkan::Ray& ray);

		/* The bound SceneNodes transform will be synchronized with the SceneEntity in Update() */
		void BindNode(const SharedPtr<SceneNode>& node, SceneEntity* entity);
	private:
		vector<SceneEntity*> mEntities;
		vector<SceneComponent*> mActiveComponents;
		map<SceneNode*, BoundNode> mBoundNodes;
	};
}
