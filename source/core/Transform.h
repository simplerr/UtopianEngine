#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>

namespace Utopian
{
	class Transform
	{
	public:
		Transform(const glm::vec3& position);
		Transform();
		~Transform();

		void SetPosition(const glm::vec3& position);
		void SetRotation(const glm::vec3& rotation);
		void SetScale(const glm::vec3& scale);

		void AddTranslation(const glm::vec3& translation);
		void AddRotation(float x, float y, float z);
		void AddScale(float x, float y, float z);
		void AddRotation(const glm::vec3& rotation);
		void AddScale(const glm::vec3& scale);

		const glm::vec3& GetPosition() const;
		const glm::vec3& GetRotation() const;
		const glm::vec3& GetScale() const;
		const glm::mat4& GetWorldMatrix() const;
		const glm::mat4& GetWorldInverseTransposeMatrix() const;

		void RebuildWorldMatrix();
	//private:

		glm::mat4 mWorld;
		glm::vec3 mPosition;
		glm::vec3 mRotation;
		glm::vec3 mScale;
	};

}
