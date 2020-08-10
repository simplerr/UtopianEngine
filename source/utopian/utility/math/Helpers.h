#pragma once
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace Utopian::Math
{
	// Retrieves the translation from a transformation matrix
	glm::vec3 GetTranslation(glm::mat4 world);

	// Retrieves the quaternion from a transformation matrix
	glm::quat GetQuaternion(const glm::mat4& transform);

	// Sets the translation in a transformation matrix
	glm::mat4 SetTranslation(glm::mat4 world, glm::vec3 translation);

	// Returns a random float
	float GetRandom(float min, float max);

	// Returns a random integer
	int32_t GetRandom(int32_t min, int32_t max);

	// Returns a random vec3
	glm::vec3 GetRandomVec3(float min, float max);
}
