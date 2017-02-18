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
#include "ecs/components/PhysicsComponent.h"
#include "ecs/EntityManager.h"
#include "ecs/Entity.h"
#include "PickingSystem.h"
#include "Camera.h"
#include "Collision.h"

// [TODO] Move
#define WINDOW_WIDTH 1600.0f
#define WINDOW_HEIGHT 1200.0f

namespace ECS
{
	PickingSystem::PickingSystem(EntityManager* entityManager, VulkanLib::Camera* camera, VulkanLib::VulkanApp* vulkanApp)
		: System(entityManager, Type::MESH_COMPONENT | Type::TRANSFORM_COMPONENT | Type::HEALTH_COMPONENT)
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
				break;
			}
		}
	}

	void PickingSystem::Process()
	{

	}

	void PickingSystem::PerformPicking()
	{
		VulkanLib::Ray ray = mCamera->GetPickingRay();

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
			//if (mEntities[pickedId].healthComponent != nullptr)
			//{
			//	mEntities[pickedId].healthComponent->SetHealth(0);
			//}
			Entity* pickedEntity = mEntities[pickedId].entity;
			if (pickedEntity->HasComponent(Type::PHYSICS_COMPONENT))
			{
				GetEntityManager()->RemoveComponent(pickedEntity, Type::PHYSICS_COMPONENT);
				mEntities[pickedId].healthComponent->SetHealth(0);
			}
			else
			{
				uint32_t maxSpeed = 2;
				uint32_t maxRotation = 100;
				float divider = 90.0f;
				ECS::PhysicsComponent* physicsComponent = new ECS::PhysicsComponent();
				physicsComponent->SetVelocity(glm::vec3(rand() % maxSpeed, rand() % maxSpeed, rand() % maxSpeed));
				physicsComponent->SetRotationSpeed(glm::vec3((rand() % maxRotation) / divider, (rand() % maxRotation) / divider, (rand() % maxRotation) / divider));
				physicsComponent->SetScaleSpeed(glm::vec3(0.0f));
				GetEntityManager()->AddComponent(mEntities[pickedId].entity, physicsComponent);
			}
		}
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

	bool PickingSystem::Contains(Entity* entity)
	{
		for (EntityCache entityCache : mEntities)
		{
			if (entityCache.entity->GetId() == entity->GetId())
				return true;
		}

		return false;
	}
}