#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "System.h"
#include "Collision.h"

namespace VulkanLib
{
	class VulkanApp;
	class Camera;
}

namespace ECS
{

	class TransformComponent;
	class MeshComponent;

	class PickingSystem : public System
	{
		struct EntityCache
		{
			Entity* entity;
			TransformComponent* transform;
			MeshComponent* mesh;
		};

	public:
		PickingSystem(VulkanLib::Camera* camera, VulkanLib::VulkanApp* vulkanApp);
		void AddEntity(Entity* entity);
		void Process();

		Entity* GetPickedEntity();

		VulkanLib::Ray GetPickingRay(VulkanLib::Camera* camera);
	private:
		std::vector<EntityCache> mEntities;
		VulkanLib::VulkanApp* mVulkanApp;
		VulkanLib::Camera* mCamera;
	};
}
