#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>

using namespace glm;

namespace Scene
{
	class Transform
	{
	public:
		Transform(const vec3& position);
		Transform();
		~Transform();

		void SetPosition(const vec3& position);
		void SetRotation(const vec3& rotation);
		void SetScale(const vec3& scale);

		void AddTranslation(const vec3& translation);
		void AddRotation(float x, float y, float z);
		void AddScale(float x, float y, float z);
		void AddRotation(const vec3& rotation);
		void AddScale(const vec3& scale);

		const vec3& GetPosition() const;
		const vec3& GetRotation() const;
		const vec3& GetScale() const;
		const mat4& GetWorldMatrix() const;
		const mat4& GetWorldInverseTransposeMatrix() const;

		void RebuildWorldMatrix();
	//private:

		mat4 mWorld;
		vec3 mPosition;
		vec3 mRotation;
		vec3 mScale;
	};

}
