#pragma once

#include <glm/glm.hpp>

namespace VulkanLib
{
	class BoundingBox
	{
	public:
		BoundingBox() {

		}

		BoundingBox(glm::vec3 min, glm::vec3 max) {
			mLocalMin = min;
			mLocalMax = max;
		}

		void Translate(glm::mat4 worldMatrix) {
			mWorldMin = glm::vec4(mLocalMin, 1.0f) * worldMatrix;
			mWorldMax = glm::vec4(mWorldMax, 1.0f) * worldMatrix;
		}

		float GetWidth() {
			return mWorldMax.x - mWorldMin.x;
		}

		float GetHeight() {
			return mWorldMax.y - mWorldMin.y;
		}

		float GetDepth() {
			return mWorldMax.z - mWorldMin.z;
		}

		glm::vec3 GetMin() {
			return mWorldMin;
		}

		glm::vec3 GetMax() {
			return mWorldMax;
		}
		
	private:
		glm::vec3 mLocalMin;
		glm::vec3 mLocalMax;

		glm::vec3 mWorldMin;
		glm::vec3 mWorldMax;
	};

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

	// Frustum
}
