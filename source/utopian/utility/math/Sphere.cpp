#include "Sphere.h"

namespace Utopian
{
   Sphere::Sphere(glm::vec3 position, float radius)
   {
      this->position = position;
      this->radius = radius;
   }

   bool Sphere::RayIntersection(const Ray& ray, float& dist)
   {
      float t0, t1;

      glm::vec3 L = position - ray.origin;
      float tca = glm::dot(L, ray.direction);
      if (tca < 0)
         return false;

      float d2 = glm::dot(L, L) - tca * tca;

      if (d2 > pow(radius, 2))
         return false;

      float thc = sqrt(pow(radius, 2) - d2);
      t0 = tca - thc;
      t1 = tca + thc;

      if (t0 < 0)
      {
         float tmp = t0;
         t0 = t1;
         t1 = tmp;

         if (t0 < 0)
            return false;
      }

      if (t0 > 0.0 && t0 < t1)
      {
         dist = t0;
         return true;
      }

      return false;
   }
}