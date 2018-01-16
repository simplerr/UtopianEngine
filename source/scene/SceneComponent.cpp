#include "SceneComponent.h"
#include "SceneEntity.h"

namespace Scene
{
	SceneComponent::SceneComponent(SceneEntity* parent)
		: mParent(parent)
	{
		SetActive(true);
	}

	SceneComponent::~SceneComponent()
	{

	}
}