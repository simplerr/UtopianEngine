#include "scene/SceneNode.h"

namespace Scene
{

	SceneNode::SceneNode()
	{
		mDrawBoundingBox = false;
		mDrawBoundingBox = true;
	}

	SceneNode::~SceneNode()
	{

	}

	void SceneNode::SetTransform(const Transform& transform)
	{
		mTransform = transform;
	}

	void SceneNode::SetPosition(const vec3& position)
	{
		mTransform.SetPosition(position);
	}

	void SceneNode::SetRotation(const vec3& rotation)
	{
		mTransform.SetRotation(rotation);
	}

	void SceneNode::SetScale(const vec3& scale)
	{
		mTransform.SetScale(scale);
	}

	void SceneNode::AddTranslation(const vec3& translation)
	{
		mTransform.AddTranslation(translation);
	}

	void SceneNode::AddRotation(const vec3& rotation)
	{
		mTransform.AddRotation(rotation);
	}

	void SceneNode::AddScale(const vec3& scale)
	{
		mTransform.AddScale(scale);
	}

	void SceneNode::SetDrawBoundingBox(bool draw)
	{
		mDrawBoundingBox = draw;
	}

	const Transform& SceneNode::GetTransform() const
	{
		return mTransform;
	}

	const vec3& SceneNode::GetPosition() const
	{
		return mTransform.GetPosition();
	}

	const vec3& SceneNode::GetRotation() const
	{
		return mTransform.GetRotation();
	}

	const vec3& SceneNode::GetScale() const
	{
		return mTransform.GetScale();
	}

	const mat4& SceneNode::GetWorldMatrix() const
	{
		return mTransform.GetWorldMatrix();
	}

	bool SceneNode::IsBoundingBoxVisible() const
	{
		return mDrawBoundingBox;
	}
}