#include "scene/SceneNode.h"

namespace Scene
{

	SceneNode::SceneNode()
	{

	}

	SceneNode::~SceneNode()
	{

	}

	void SceneNode::SetTransform(const Transform& transform)
	{
		mTransform = transform;
	}

	const Transform& SceneNode::GetTransform() const
	{
		return mTransform;
	}
}