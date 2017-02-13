#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <Windows.h>
#include <float.h>
#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/VulkanApp.h"
#include "vulkan/VulkanDebug.h"
#include "ecs/components/TransformComponent.h"
#include "ecs/components/MeshComponent.h"
#include "ecs/components/HealthComponent.h"
#include "ecs/Entity.h"
#include "PickingSystem.h"
#include "Camera.h"
#include "Collision.h"

// [TODO] Move
#define WINDOW_WIDTH 1600.0f
#define WINDOW_HEIGHT 1200.0f

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
		entityCache.healthComponent = dynamic_cast<HealthComponent*>(entity->GetComponent(HEALTH_COMPONENT));

		VulkanLib::VulkanDebug::ConsolePrint(entityCache.transform->GetPosition());

		mEntities.push_back(entityCache);
	}

	void PickingSystem::RemoveEntity(Entity* entity)
	{
		for (auto iter = mEntities.begin(); iter < mEntities.end(); iter++)
		{
			if ((*iter).entity->GetId() == entity->GetId())
			{
				iter = mEntities.erase(iter);
			}
		}
	}

	void PickingSystem::Process()
	{

	}

	void PickingSystem::PerformPicking()
	{
		VulkanLib::Ray ray = GetPickingRay(mCamera);

		static  int test = 0;
		test++;
		VulkanLib::VulkanDebug::ConsolePrint("test: " + std::to_string(test));
		float minDist = FLT_MAX;
		uint32_t pickedId = -1;
		for (int i = 0; i < mEntities.size(); i++)
		{
			glm::vec3 pos = mEntities[i].transform->GetPosition();
			VulkanLib::BoundingBox boundingBox = mEntities[i].mesh->GetBoundingBox();

			boundingBox.Update(mEntities[i].transform->GetWorldMatrix());
			//VulkanLib::VulkanDebug::ConsolePrint(boundingBox.GetMin(), "***** picked min aabb: ");

			float dist = FLT_MAX;
			VulkanLib::Sphere sphere(pos, 200);
			//if(sphere.RayIntersection(ray, dist))
			if (boundingBox.RayIntersect(ray, dist))
			{
				VulkanLib::VulkanDebug::ConsolePrint(pos, "picked pos: ");
				VulkanLib::VulkanDebug::ConsolePrint(boundingBox.GetMin(), "picked min aabb: ");
				VulkanLib::VulkanDebug::ConsolePrint("Picked entity ID: " + std::to_string(i));
				//if (dist < minDist)
				//{
				//	minDist = dist;
				//	minId = i;
				//}

				pickedId = i;
				//break;
			}
		}

		if(pickedId != -1)
		{
			// Do something with the entity that was picked
			if (mEntities[pickedId].healthComponent != nullptr)
			{
				mEntities[pickedId].healthComponent->SetHealth(0);
			}
		}
	}

	VulkanLib::Ray PickingSystem::GetPickingRay(VulkanLib::Camera* camera)
	{
		// Camera/view matrix
		mat4 viewMatrix = camera->GetView();
		mat4 projectionMatrix = camera->GetProjection();

		mat4 inverseView = glm::inverse(viewMatrix);
		mat4 inverseProjection = glm::inverse(projectionMatrix);

		// [TODO] Move
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		ScreenToClient(mVulkanApp->GetWindow()->GetHwnd(), &cursorPos);

		float vx = (+2.0f * cursorPos.x / WINDOW_WIDTH - 1.0f);
		float vy = (-2.0f * cursorPos.y / WINDOW_HEIGHT + 1.0f);

		vec4 rayDir = inverseProjection * vec4(-vx, vy, 1.0, 1.0);
		rayDir.z = 1.0;
		rayDir.w = 0;
		rayDir = inverseView * rayDir;
		//rayDir = rayDir / rayDir.w;
		//rayDir -= vec4(camera->GetPosition(), 0);
		vec3 rayFinalDir = glm::normalize(vec3(rayDir.x, rayDir.y, rayDir.z));

		return VulkanLib::Ray(camera->GetPosition(), rayFinalDir);
	}

	void PickingSystem::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
			PerformPicking();
			break;
		}
	}

	Entity* PickingSystem::GetPickedEntity()
	{
		return nullptr;
	}
}