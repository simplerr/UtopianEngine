#include  "Collision.h"

namespace VulkanLib
{
	void BoundingBox::Init(const std::vector<Vertex>& vertices)
	{
		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(-FLT_MAX);

		for (int i = 0; i < vertices.size(); i++)
		{
			Vertex vertex = vertices[i];

			if (vertex.Pos.x < min.x)
				min.x = vertex.Pos.x;
			else if (vertex.Pos.x > max.x)
				max.x = vertex.Pos.x;

			if (vertex.Pos.y < min.y)
				min.y = vertex.Pos.y;
			else if (vertex.Pos.y > max.y)
				max.y = vertex.Pos.y;

			if (vertex.Pos.z < min.z)
				min.z = vertex.Pos.z;
			else if (vertex.Pos.z > max.z)
				max.z = vertex.Pos.z;
		}

		mMin = min;
		mMax = max;

		// Front
		mVertices.push_back(glm::vec3(min.x, min.y, max.z));
		mVertices.push_back(glm::vec3(max.x, min.y, max.z));
		mVertices.push_back(glm::vec3(max.x, max.y, max.z));
		mVertices.push_back(glm::vec3(min.x, max.y, max.z));

		// Back
		mVertices.push_back(glm::vec3(min.x, min.y, min.z));
		mVertices.push_back(glm::vec3(max.x, min.y, min.z));
		mVertices.push_back(glm::vec3(max.x, max.y, min.z));
		mVertices.push_back(glm::vec3(min.x, max.y, min.z));
	}

	void BoundingBox::Update(glm::mat4 worldMatrix)
	{
		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(-FLT_MAX);

		for (int i = 0; i < mVertices.size(); i++)
		{
			glm::vec3 pos = worldMatrix * glm::vec4(mVertices[i], 1.0f);

			if (pos.x < min.x)
				min.x = pos.x;
			else if (pos.x > max.x)
				max.x = pos.x;

			if (pos.y < min.y)
				min.y = pos.y;
			else if (pos.y > max.y)
				max.y = pos.y;

			if (pos.z < min.z)
				min.z = pos.z;
			else if (pos.z > max.z)
				max.z = pos.z;
		}

		mMin = min;
		mMax = max;
	}

	bool BoundingBox::RayIntersect(const Ray& ray, float& dist)
	{
		glm::vec3 min = mMin;
		glm::vec3 max = mMax;


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

	float BoundingBox::GetWidth() {
		return mMax.x - mMin.x;
	}

	float BoundingBox::GetHeight() {
		return mMax.y - mMin.y;
	}

	float BoundingBox::GetDepth() {
		return mMax.z - mMin.z;
	}

	glm::vec3 BoundingBox::GetMin() const {
		return mMin;
	}

	glm::vec3 BoundingBox::GetMax() const {
		return mMax;
	}

	Sphere::Sphere(glm::vec3 position, float radius) 
	{
		this->position = position;
		this->radius = radius;
	}

	bool Sphere::RayIntersection(const Ray& ray, float& dist)
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
}