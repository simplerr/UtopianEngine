#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <utility>
#include <vector>
#include <glm/glm.hpp>
#include "vulkan/Vertex.h"
#include "Ray.h"

namespace Utopian
{
	class BoundingBox
	{
	public:
		BoundingBox() {}

		void Init(const std::vector<Vk::Vertex>& vertices);
		void Init(glm::vec3 position, glm::vec3 extents);
		void Update(glm::mat4 worldMatrix);
		bool RayIntersect(const Ray& ray, float& dist);

		float GetWidth() const;
		float GetHeight() const;
		float GetDepth() const;
		glm::vec3 GetMin() const;
		glm::vec3 GetMax() const;

	private:
		std::vector<glm::vec3> mVertices;

		glm::vec3 mMin;
		glm::vec3 mMax;
	};
}