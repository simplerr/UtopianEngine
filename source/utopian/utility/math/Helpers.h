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
   uint64_t GetRandom(uint64_t min = 0u, uint64_t max = std::numeric_limits<std::uint64_t>::max());

   // Returns a random vec3
   glm::vec3 GetRandomVec3(float min, float max);

   // Returns the distance from point to line
   float DistanceToLine(glm::vec3 lineStart, glm::vec3 lineEnd, glm::vec3 point);
}
