#pragma once
#include <vector>
#include "scene/SceneComponent.h"
#include "scene/Module.h"
#include "Common.h"

namespace Scene
{
	class SceneEntity;

	class SceneManager : public Module<SceneManager>
	{
	public:
		SceneManager();
		~SceneManager();
	
		void Update();
		void NotifyComponentCreated(SceneComponent* component);
	private:
		vector<SceneComponent*> mActiveComponents;
	};
}
