#pragma once
#include <vector>
#include <map>
#include "scene/SceneComponent.h"
#include "scene/SceneNode.h"
#include "utility/Module.h"
#include "utility/Common.h"

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
		void NotifyComponentCreated(SceneComponent* component);

		/* The bound SceneNodes transform will be synchronized with the SceneEntity in Update() */
		void BindNode(const SharedPtr<SceneNode>& node, SceneEntity* entity);
	private:
		vector<SceneComponent*> mActiveComponents;
		map<SceneNode*, BoundNode> mBoundNodes;
	};
}
