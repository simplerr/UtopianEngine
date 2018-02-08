#include "SceneComponent.h"
#include "Actor.h"

namespace Utopian
{
	SceneComponent::SceneComponent(Actor* parent)
		: mParent(parent)
	{
		SetActive(true);
	}

	SceneComponent::~SceneComponent()
	{

	}

	const BoundingBox SceneComponent::GetBoundingBox() const
	{
		BoundingBox boundingBox;

		boundingBox.Init(vec3(0.0f), vec3(0.0f));

		return boundingBox;
	}
}