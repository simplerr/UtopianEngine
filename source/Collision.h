#pragma once

#include <utility>
#include <glm/glm.hpp>

namespace VulkanLib
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
		BoundingBox() {

		}

		BoundingBox(glm::vec3 min, glm::vec3 max) {
			mLocalMin = min;
			mLocalMax = max;
		}

		void Translate(glm::mat4 worldMatrix) {
			mWorldMin = glm::vec4(mLocalMin, 1.0f) * worldMatrix;
			mWorldMax = glm::vec4(mLocalMax, 1.0f) * worldMatrix;
		}

		bool RayIntersect(const Ray& ray, float& dist)
		{
			glm::vec3 min = mWorldMin;
			glm::vec3 max = mWorldMax;


			float tmin = (min.x - ray.origin.x) / ray.direction.x;
			float tmax = (max.x - ray.origin.x) / ray.direction.x;

			if (tmin > tmax)
				std::swap(tmin, tmax);

			float tymin = (min.y - ray.origin.y) / ray.direction.y;
			float tymax = (max.y - ray.origin.y) / ray.direction.y;

			if (tymin > tymax)
				std::swap(tymin, tymax);

			if ((tmin > tymax) || (tymin > tmax))
				return false;

			if (tymin > tmin)
				tmin = tymin;

			if (tymax < tmax)
				tmax = tymax;

			float tzmin = (min.z - ray.origin.z) / ray.direction.z;
			float tzmax = (max.z - ray.origin.z) / ray.direction.z;

			if (tzmin > tzmax)
				std::swap(tzmin, tzmax);

			if ((tmin > tzmax) || (tzmin > tmax))
				return false;

			if (tzmin > tmin)
				tmin = tzmin;

			if (tzmax < tmax)
				tmax = tzmax;

			return true;
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

		glm::vec3 GetMin() const {
			return mWorldMin;
		}

		glm::vec3 GetMax() const {
			return mWorldMax;
		}
		
	private:
		glm::vec3 mLocalMin;
		glm::vec3 mLocalMax;

		glm::vec3 mWorldMin;
		glm::vec3 mWorldMax;
	};

	class Sphere
	{
	public:
		Sphere(glm::vec3 position, float radius) {
			this->position = position;
			this->radius = radius;
		}

		bool RayIntersection(const Ray& ray, float& dist)
		{
			float t0, t1;

			glm::vec3 L = position - ray.origin;
			float tca = glm::dot(L, ray.direction);
			if (tca < 0)
				return false;

			float d2 = glm::dot(L, L) - tca * tca;

			if (d2 > pow(radius, 2))
				return false;

			float thc = sqrt(pow(radius, 2) - d2);
			t0 = tca - thc;
			t1 = tca + thc;

			if (t0 < 0)
			{
				float tmp = t0;
				t0 = t1;
				t1 = tmp;

				if (t0 < 0)
					return false;
			}

			if (t0 > 0.0 && t0 < t1)
			{
				dist = t0;
				return true;
			}

			return false;
		}

	private:
		glm::vec3 position;
		float radius;
	};

	// Frustum
}
