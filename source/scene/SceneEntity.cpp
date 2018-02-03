#include "SceneEntity.h"
#include "scene/ObjectManager.h"
#include "scene/CTransform.h"
#include "ecs/SystemManager.h"
#include "Exception.h"

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
		World::Instance().NotifyEntityCreated(entity.get());

		// Let SceneManager assign root node

		return entity;
	}

	Vulkan::BoundingBox SceneEntity::GetBoundingBox() const
	{
		Vulkan::BoundingBox boundingBox;
		boundingBox.Init(GetTransform().GetPosition(), glm::vec3(10000.0f));

		return boundingBox;
	}

	const Transform& SceneEntity::GetTransform() const
	{
		CTransform* transform = GetComponent<CTransform>();

		if (transform == nullptr)
			THROW_EXCEPTION(Exception, "No CTransform component");

		return transform->GetTransform();
	}
}