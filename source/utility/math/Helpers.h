#pragma once
#include <glm/glm.hpp>

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
}
