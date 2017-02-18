#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vector>
#include <glm/glm.hpp>
#include <window.h>
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
	class HealthComponent;

	class PickingSystem : public System
	{
		struct EntityCache
		{
			Entity* entity;
			TransformComponent* transform;
			MeshComponent* mesh;
			HealthComponent* healthComponent;
		};

	public:
		PickingSystem(EntityManager* entityManager, VulkanLib::Camera* camera, VulkanLib::VulkanApp* vulkanApp);
		void AddEntity(Entity* entity);
		void RemoveEntity(Entity* entity);
		void Process();
		void PerformPicking();

		Entity* GetPickedEntity();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		bool Contains(Entity* entity);
	private:
		std::vector<EntityCache> mEntities;
		VulkanLib::VulkanApp* mVulkanApp;
		VulkanLib::Camera* mCamera;
	};
}
