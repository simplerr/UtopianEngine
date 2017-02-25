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
	public:
		PickingSystem(EntityManager* entityManager, VulkanLib::Camera* camera);
		void Process();
		void PerformPicking();

		Entity* GetPickedEntity();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		VulkanLib::Camera* mCamera;
	};
}
