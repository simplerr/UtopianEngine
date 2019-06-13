#pragma once
#include "utility/math/Helpers.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace Utopian::Math
{
	glm::vec3 GetTranslation(glm::mat4 world)
	{
		glm::vec3 translation;
		translation.x = world[3][0];
		translation.y = world[3][1];
		translation.z = world[3][2];

		return translation;
	}

	glm::mat4 SetTranslation(glm::mat4 world, glm::vec3 translation)
	{
		world[3][0] = translation.x;
		world[3][1] = translation.y;
		world[3][2] = translation.z;

		return world;
	}

	// Retrieves the quaternion from a transformation matrix
	glm::quat GetQuaternion(const glm::mat4& transform)
	{
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat rotation;
		glm::decompose(glm::mat4(transform), scale, rotation, translation, skew, perspective);

		return rotation;
	}
}
