#include "Component.h"
#include "Actor.h"

namespace Utopian
{
	Component::Component(Actor* parent)
		: mParent(parent)
	{
		SetActive(true);
	}

	Component::~Component()
	{

	}

	const BoundingBox Component::GetBoundingBox() const
	{
		BoundingBox boundingBox;

		boundingBox.Init(glm::vec3(0.0f), glm::vec3(0.0f));

		return boundingBox;
	}
}