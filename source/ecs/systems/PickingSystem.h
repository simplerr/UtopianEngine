#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "System.h"

namespace VulkanLib
{
	class VulkanApp;
	class Camera;
}

namespace ECS
{
	struct Ray
	{
		Ray() {}
		Ray(glm::vec3 o, glm::vec3 d) : origin(o), direction(d) {}
		glm::vec3 origin;
		glm::vec3 direction;
	};

	struct Sphere
	{
		Sphere(glm::vec3 position, float radius) {
			this->position = position;
			this->radius = radius;
		}

		glm::vec3 position;
		float radius;
	};

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
		bool RaySphereIntersection(Ray ray, Sphere sphere, float& dist);
		//void RayBoxIntersection(Ray ray, BoundingBox box, float& dist);

		Ray GetPickingRay(VulkanLib::Camera* camera);
	private:
		std::vector<EntityCache> mEntities;
		VulkanLib::VulkanApp* mVulkanApp;
		VulkanLib::Camera* mCamera;
	};
}
