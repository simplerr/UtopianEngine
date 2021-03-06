#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <utility>
#include <vector>
#include <glm/glm.hpp>
#include "vulkan/Vertex.h"
#include "Ray.h"

namespace Utopian
{
   class Sphere
   {
   public:
      Sphere(glm::vec3 position, float radius);
      bool RayIntersection(const Ray& ray, float& dist);

   private:
      glm::vec3 position;
      float radius;
   };
}