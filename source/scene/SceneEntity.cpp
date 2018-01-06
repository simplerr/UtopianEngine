#include "SceneEntity.h"
#include "scene/ObjectManager.h"
#include "ecs/SystemManager.h"

namespace Scene
{
	SceneEntity::SceneEntity(string name)
		: Object(name)
	{

	}

	SharedPtr<SceneEntity> SceneEntity::Create(string name)
	{
		SharedPtr<SceneEntity> entity(new SceneEntity(name));

		ObjectManager::Instance().RegisterObject(entity);

		// Let SceneManager assign root node

		return entity;
	}
}