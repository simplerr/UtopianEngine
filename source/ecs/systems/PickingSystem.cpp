#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <Windows.h>
#include <float.h>
#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/Renderer.h"
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
	PickingSystem::PickingSystem(EntityManager* entityManager, VulkanLib::Camera* camera)
		: System(entityManager, Type::MESH_COMPONENT | Type::TRANSFORM_COMPONENT | Type::HEALTH_COMPONENT)
	{
		mCamera = camera;
	}

	void PickingSystem::Process()
	{

	}

	void PickingSystem::PerformPicking()
	{
		VulkanLib::Ray ray = mCamera->GetPickingRay();

		float minDist = FLT_MAX;
		uint32_t pickedId = -1;
		for (int i = 0; i < mEntities.size(); i++)
		{
			glm::vec3 pos = mEntities[i].transformComponent->GetPosition();
			VulkanLib::BoundingBox boundingBox = mEntities[i].meshComponent->GetBoundingBox();

			boundingBox.Update(mEntities[i].transformComponent->GetWorldMatrix());
			//VulkanLib::VulkanDebug::ConsolePrint(boundingBox.GetMin(), "***** picked min aabb: ");

			float dist = FLT_MAX;
			VulkanLib::Sphere sphere(pos, 200);
			//if(sphere.RayIntersection(ray, dist))
			if (boundingBox.RayIntersect(ray, dist))
			{
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
				//mEntities[pickedId].healthComponent->SetHealth(0);
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
}