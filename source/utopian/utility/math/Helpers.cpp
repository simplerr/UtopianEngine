#include "utility/math/Helpers.h"
#include <random>

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

   float GetRandom(float min, float max)
   {
      min = glm::min(min, max);
      max = glm::max(min, max);

      std::random_device rd;
      std::mt19937 mt(rd());
      std::uniform_real_distribution<float> dist(min, max);
      float random = dist(mt);

      return random;
   }

   uint64_t GetRandom(uint64_t min, uint64_t max)
   {
      min = glm::min(min, max);
      max = glm::max(min, max);

      std::random_device rd;
      std::mt19937 mt(rd());
      std::uniform_int_distribution<uint64_t> dist(min, max);
      uint64_t random = (uint64_t)dist(mt);

      return random;
   }

   glm::vec3 GetRandomVec3(float min, float max)
   {
      min = glm::min(min, max);
      max = glm::max(min, max);

      std::random_device rd;
      std::mt19937 mt(rd());
      std::uniform_real_distribution<float> dist(min, max);
      glm::vec3 random = glm::vec3(dist(mt), dist(mt), dist(mt));

      return random;
   }

   float DistanceToLine(glm::vec3 lineStart, glm::vec3 lineEnd, glm::vec3 point)
   {
      // Reference: http://geomalgorithms.com/a02-_lines.html
      glm::vec3 v = lineEnd - lineStart;
      glm::vec3 w = point - lineStart;

      float c1 = glm::dot(w, v);
      float c2 = glm::dot(v, v);
      float b = c1 / c2;

      glm::vec3 Pb = lineStart + b * v;

      glm::vec3 delta = point - Pb;
      return glm::sqrt(glm::dot(delta, delta));
   }
}
