#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <utility>
#include <vector>
#include <glm/glm.hpp>
#include "vulkan/Vertex.h"

namespace Utopian
{
	struct Ray
	{
		Ray() {}
		Ray(glm::vec3 o, glm::vec3 d) : origin(o), direction(d) {}
		glm::vec3 origin;
		glm::vec3 direction;
	};
}