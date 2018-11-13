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
		Actor* actor;
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
		void AddActor(const SharedPtr<Actor>& actor);
		void AddComponent(const SharedPtr<Component>& component);

		void RemoveActor(Actor* actor);

		Actor* RayIntersection(const Ray& ray);
		vector<SharedPtr<Actor>>& GetActors();

		/* The bound SceneNodes transform will be synchronized with the Sceneactor in Update() */
		void BindNode(const SharedPtr<SceneNode>& node, Actor* actor);
		void RemoveNode(const SharedPtr<SceneNode>& node);
	private:
		vector<SharedPtr<Actor>> mActors;
		vector<SharedPtr<Component>> mComponents;
		map<SceneNode*, BoundNode> mBoundNodes;
	};
}
