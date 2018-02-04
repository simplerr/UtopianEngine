#include "SceneComponent.h"
#include "Actor.h"

namespace Scene
{
	SceneComponent::SceneComponent(Actor* parent)
		: mParent(parent)
	{
		SetActive(true);
	}

	SceneComponent::~SceneComponent()
	{

	}

	const Vulkan::BoundingBox SceneComponent::GetBoundingBox() const
	{
		Vulkan::BoundingBox boundingBox;

		boundingBox.Init(vec3(0.0f), vec3(0.0f));

		return boundingBox;
	}
}