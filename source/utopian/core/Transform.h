#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Utopian
{
	class Transform
	{
	public:
		Transform(const glm::vec3& position);
		Transform();
		~Transform();

		void SetPosition(const glm::vec3& position);
		void SetRotation(const glm::vec3& eulerRotation);
		void SetScale(const glm::vec3& scale);
		void SetOrientation(const glm::quat& orientation);

		void AddTranslation(const glm::vec3& translation);
		void AddRotation(const glm::vec3& eulerRotation, bool local = false);
		void AddScale(const glm::vec3& scale);

		const glm::vec3& GetPosition() const;
		const glm::vec3& GetScale() const;
		const glm::mat4& GetWorldMatrix() const;
		const glm::quat& GetOrientation() const;
		glm::mat4 GetWorldInverseTransposeMatrix() const;

		void RebuildWorldMatrix();
	//private:

		glm::mat4 mWorld;
		glm::vec3 mPosition;
		glm::vec3 mScale;
		glm::quat mOrientation;

	private:
		glm::quat OrientationFromEuler(const glm::vec3& eulerRotation);
	};

}
