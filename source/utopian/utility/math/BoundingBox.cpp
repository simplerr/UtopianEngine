#include "BoundingBox.h"

namespace Utopian
{
	void BoundingBox::Init(const std::vector<Vk::Vertex>& vertices)
	{
		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(-FLT_MAX);

		for (int i = 0; i < vertices.size(); i++)
		{
			Vk::Vertex vertex = vertices[i];

			// Note: Todo: The physical representation does not match
			// the one used for rendering.
			vertex.Pos = vertex.Pos *= -1;

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

	void BoundingBox::Init(glm::vec3 position, glm::vec3 extents)
	{
		mMin = position;
		mMax = position + extents;

		// Front
		mVertices.push_back(glm::vec3(mMin.x, mMin.y, mMax.z));
		mVertices.push_back(glm::vec3(mMax.x, mMin.y, mMax.z));
		mVertices.push_back(glm::vec3(mMax.x, mMax.y, mMax.z));
		mVertices.push_back(glm::vec3(mMin.x, mMax.y, mMax.z));

		// Back
		mVertices.push_back(glm::vec3(mMin.x, mMin.y, mMin.z));
		mVertices.push_back(glm::vec3(mMax.x, mMin.y, mMin.z));
		mVertices.push_back(glm::vec3(mMax.x, mMax.y, mMin.z));
		mVertices.push_back(glm::vec3(mMin.x, mMax.y, mMin.z));
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

	bool BoundingBox::RayIntersect(const Ray& ray, float& dist, glm::vec3& normal)
	{
		glm::vec3 min = mMin;
		glm::vec3 max = mMax;

		float left = (min.x - ray.origin.x) / ray.direction.x;
		float right = (max.x - ray.origin.x) / ray.direction.x;

		float tmin = left;
		float tmax = right;

		if (tmin > tmax)
			std::swap(tmin, tmax);

		float bottom = (min.y - ray.origin.y) / ray.direction.y;
		float top = (max.y - ray.origin.y) / ray.direction.y;

		float tymax = top;
		float tymin = bottom;

		if (tymin > tymax)
			std::swap(tymin, tymax);

		if ((tmin > tymax) || (tymin > tmax))
			return false;

		if (tymin > tmin)
			tmin = tymin;

		if (tymax < tmax)
			tmax = tymax;

		float front = (min.z - ray.origin.z) / ray.direction.z;
		float back = (max.z - ray.origin.z) / ray.direction.z;

		float tzmin = front;
		float tzmax = back;

		if (tzmin > tzmax)
			std::swap(tzmin, tzmax);

		if ((tmin > tzmax) || (tzmin > tmax))
			return false;

		if (tzmin > tmin)
			tmin = tzmin;

		if (tzmax < tmax)
			tmax = tzmax;

		dist = tmin;

		// Get the normal
		if (tmin == left)
			normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		else if (tmin == right)
			normal = glm::vec3(1.0f, 0.0f, 0.0f);
		else if (tmin == top)
			normal = glm::vec3(0.0f, 1.0f, 0.0f);
		else if (tmin == bottom)
			normal = glm::vec3(0.0f, -1.0f, 0.0f);
		else if (tmin == front)
			normal = glm::vec3(0.0f, 0.0f, -1.0f);
		else if (tmin == back)
			normal = glm::vec3(0.0f, 0.0f, 1.0f);

		return true;
	}

	float BoundingBox::GetWidth() const {
		return mMax.x - mMin.x;
	}

	float BoundingBox::GetHeight() const {
		return mMax.y - mMin.y;
	}

	float BoundingBox::GetDepth() const {
		return mMax.z - mMin.z;
	}

	float BoundingBox::GetRadius() const
	{
		return glm::max(GetWidth(), glm::max(GetHeight(), GetDepth())) / 2.0f;
	}

	glm::vec3 BoundingBox::GetMin() const {
		return mMin;
	}

	glm::vec3 BoundingBox::GetMax() const {
		return mMax;
	}
}