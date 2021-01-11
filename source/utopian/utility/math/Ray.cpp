#include "Ray.h"

namespace Utopian
{
   Ray::Ray()
   {

   }

   Ray::Ray(glm::vec3 origin, glm::vec3 direction)
   {
      this->origin = origin;
      this->direction = direction;
   }

   bool Ray::TriangleIntersect(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, glm::vec3& intersectPoint, float &distance)
   {
      const float EPSILON = 0.0000001f;
      glm::vec3 edge1, edge2, h, s, q;
      float a, f, u, v;
      edge1 = v1 - v0;
      edge2 = v2 - v0;
      h = glm::cross(direction, edge2);
      a = glm::dot(edge1, h);
      if (a > -EPSILON && a < EPSILON)
         return false; // This ray is parallel to this triangle

      f = 1.0f / a;
      s = origin - v0;
      u = f * glm::dot(s, h);
      if (u < 0.0 || u > 1.0)
         return false;

      q = glm::cross(s, edge1);
      v = f * glm::dot(direction, q);
      if (v < 0.0 || u + v > 1.0)
         return false;

      // At this stage we can compute t to find out where the intersection point is on the line
      float t = f * glm::dot(edge2, q);
      if (t > EPSILON) // Ray intersection
      {
         intersectPoint = origin + direction * t;
         distance = t;
         return true;
      }
      else // This means that there is a line intersection but not a ray intersection
         return false;
   }
}