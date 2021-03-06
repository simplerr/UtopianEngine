#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "LinearMath/btVector3.h"

namespace Utopian
{
   inline glm::vec3 ToVec3(const btVector3& vector)
   {
      return glm::vec3(vector.getX(), vector.getY(), vector.getZ());
   }

   inline glm::vec4 ToVec4(const btVector3& vector)
   {
      return glm::vec4(vector.getX(), vector.getY(), vector.getZ(), 1.0f);
   }

   inline glm::quat ToQuaternion(const btQuaternion& quaternion)
   {
      return glm::quat((float)quaternion.getW(), (float)quaternion.getX(), (float)quaternion.getY(), (float)quaternion.getZ());
   }

   inline btVector3 ToBulletVec3(glm::vec3 vector)
   {
      return btVector3(vector.x, vector.y, vector.z);
   }

   inline btVector4 ToBulletVec4(glm::vec4 vector)
   {
      return btVector4(vector.x, vector.y, vector.z, vector.w);
   }

   inline btQuaternion ToBulletQuaternion(const glm::quat& quaternion)
   {
      return btQuaternion(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
   }
}
