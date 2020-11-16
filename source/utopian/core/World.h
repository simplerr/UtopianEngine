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

		// The transforms from the Actors needds to update the transforms of the SceneNodes
		// CRigidBody needs the correct bounding box which only is available of they have been synchronized.
		// We can't do it in BindNode() since we can't gurantee that CTransform is added before CRenderable
		// Todo: Note: This should be handled better!
		void SynchronizeNodeTransforms();

		void RemoveActor(Actor* actor);
		void RemoveActors();
		void LoadScene();

		SharedPtr<Actor> RayIntersection(const Ray& ray, float& distance);
		std::vector<SharedPtr<Actor>>& GetActors();
		uint32_t GetActorIndex(SharedPtr<Actor> actor);

		/* The bound SceneNodes transform will be synchronized with the Sceneactor in Update() */
		void BindNode(const SharedPtr<SceneNode>& node, Actor* actor);
		void RemoveNode(const SharedPtr<SceneNode>& node);
	private:
		std::vector<SharedPtr<Actor>> mActors;
		std::vector<SharedPtr<Component>> mComponents;
		std::map<SceneNode*, BoundNode> mBoundNodes;
	};

	World& gWorld();
}
