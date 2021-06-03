#pragma once

#include <utility>
#include <vector>
#include <glm/glm.hpp>
#include "vulkan/Vertex.h"

namespace Utopian
{
   struct Ray
   {
      Ray();
      Ray(glm::vec3 origin, glm::vec3 direction);

      bool TriangleIntersect(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, glm::vec3& intersectPoint, float &distance);

      glm::vec3 origin;
      glm::vec3 direction;
   };
}