#include <Windows.h>
#include <float.h>
#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/VulkanApp.h"
#include "vulkan/VulkanDebug.h"
#include "ecs/components/TransformComponent.h"
#include "ecs/components/MeshComponent.h"
#include "ecs/Entity.h"
#include "PickingSystem.h"
#include "Camera.h"
#include "Collision.h"

// [TODO] Move
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 1200

namespace ECS
{
	PickingSystem::PickingSystem(VulkanLib::Camera* camera, VulkanLib::VulkanApp* vulkanApp)
	{
		mVulkanApp = vulkanApp;
		mCamera = camera;
	}

	void PickingSystem::AddEntity(Entity* entity)
	{
		EntityCache entityCache;
		entityCache.entity = entity;
		entityCache.transform = dynamic_cast<TransformComponent*>(entity->GetComponent(TRANSFORM_COMPONENT));
		entityCache.mesh = dynamic_cast<MeshComponent*>(entity->GetComponent(MESH_COMPONENT));

		mEntities.push_back(entityCache);
	}

	void PickingSystem::Process()
	{
		float minDist = FLT_MAX;
		uint32_t minId = 0;
		for (int i = 0; i < mEntities.size(); i++)
		{
			glm::vec3 pos = mEntities[i].transform->GetPosition();
			VulkanLib::BoundingBox boundingBox = mEntities[i].mesh->GetBoundingBox();

			boundingBox.Translate(mEntities[i].transform->GetWorldMatrix());

			float dist = FLT_MAX;
			VulkanLib::Ray ray = GetPickingRay(mCamera);
			if (boundingBox.RayIntersect(ray, dist))
			{
				VulkanLib::VulkanDebug::ConsolePrint("GET PHYSICAL " + std::to_string(dist) + "\n");
				if (dist < minDist)
				{
					minDist = dist;
					minId = i;
				}
			}
		}

		// Do something with the entity that was picked
		mEntities[minId];
	}

	VulkanLib::Ray PickingSystem::GetPickingRay(VulkanLib::Camera* camera)
	{
		// Camera/view matrix
		mat4 viewMatrix = glm::lookAt(camera->GetPosition(), camera->GetTarget(), camera->GetUp());

		// Projection matrix
		mat4 projectionMatrix = glm::perspective(90, WINDOW_WIDTH / WINDOW_HEIGHT, 1, 2);

		// Get inverse of view*proj
		mat4 inverseViewProjection = projectionMatrix * viewMatrix;
		inverseViewProjection = glm::inverse(inverseViewProjection);

		// [TODO] Move
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		ScreenToClient(mVulkanApp->GetWindow()->GetHwnd(), &cursorPos);

		float vx = (+2.0f * cursorPos.x / WINDOW_WIDTH - 1);
		float vy = (-2.0f * cursorPos.y / WINDOW_HEIGHT + 1);

		vec4 rayDir = inverseViewProjection * vec4(vx, vy, 0, 1);
		rayDir = rayDir / rayDir.w;
		rayDir -= vec4(camera->GetPosition(), 0);
		vec3 rayFinalDir = glm::normalize(vec3(rayDir.x, rayDir.y, rayDir.z));

		return VulkanLib::Ray(camera->GetPosition(), rayFinalDir);
	}

	Entity* PickingSystem::GetPickedEntity()
	{
		return nullptr;
	}
}