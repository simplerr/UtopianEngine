#include "core/SceneNode.h"

namespace Utopian
{

	SceneNode::SceneNode()
	{
		mDrawBoundingBox = false;
	}

	SceneNode::~SceneNode()
	{

	}

	void SceneNode::SetTransform(const Transform& transform)
	{
		mTransform = transform;
	}

	void SceneNode::SetPosition(const glm::vec3& position)
	{
		mTransform.SetPosition(position);
	}

	void SceneNode::SetRotation(const glm::vec3& rotation)
	{
		mTransform.SetRotation(rotation);
	}

	void SceneNode::SetScale(const glm::vec3& scale)
	{
		mTransform.SetScale(scale);
	}

	void SceneNode::SetId(uint32_t id)
	{
		mId = id;
	}

	void SceneNode::AddTranslation(const glm::vec3& translation)
	{
		mTransform.AddTranslation(translation);
	}

	void SceneNode::AddRotation(const glm::vec3& rotation)
	{
		mTransform.AddRotation(rotation);
	}

	void SceneNode::AddScale(const glm::vec3& scale)
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

	const glm::vec3& SceneNode::GetPosition() const
	{
		return mTransform.GetPosition();
	}

	const glm::vec3& SceneNode::GetScale() const
	{
		return mTransform.GetScale();
	}

	const glm::mat4& SceneNode::GetWorldMatrix() const
	{
		return mTransform.GetWorldMatrix();
	}

	bool SceneNode::IsBoundingBoxVisible() const
	{
		return mDrawBoundingBox;
	}

	uint32_t SceneNode::GetId() const
	{
		return mId;
	}
}