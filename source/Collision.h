#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <utility>
#include <vector>
#include <glm/glm.hpp>
#include "vulkan/Vertex.h"

namespace Vulkan
{
	struct Ray
	{
		Ray() {}
		Ray(glm::vec3 o, glm::vec3 d) : origin(o), direction(d) {}
		glm::vec3 origin;
		glm::vec3 direction;
	};

	class BoundingBox
	{
	public:
		BoundingBox() {}

		void Init(const std::vector<Vertex>& vertices);
		void Init(glm::vec3 position, glm::vec3 extents);
		void Update(glm::mat4 worldMatrix);
		bool RayIntersect(const Ray& ray, float& dist);

		float GetWidth();
		float GetHeight();
		float GetDepth();
		glm::vec3 GetMin() const;
		glm::vec3 GetMax() const;

	private:
		std::vector<glm::vec3> mVertices;

		glm::vec3 mMin;
		glm::vec3 mMax;
	};

	class Sphere
	{
	public:
		Sphere(glm::vec3 position, float radius);
		bool RayIntersection(const Ray& ray, float& dist);
		
	private:
		glm::vec3 position;
		float radius;
	};

	// Frustum
}
