#include "scene/SceneManager.h"
#include "scene/SceneEntity.h"

namespace Scene
{
	SceneManager::SceneManager()
	{

	}

	SceneManager::~SceneManager()
	{

	}

	void SceneManager::NotifyComponentCreated(SceneComponent* component)
	{
		component->OnCreated();

		mActiveComponents.push_back(component);
	}

	void SceneManager::Update()
	{
		for (auto& entry : mActiveComponents)
		{
			entry->Update();
		}
	}

	// onInitialized()
	// SceneManager::BindActor(mInternal, GetParent())
}