#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Transform.h"

namespace Utopian
{
   Transform::Transform(const glm::vec3& position)
   {
      SetPosition(position);
      SetRotation(glm::vec3(0, 0, 0));
      SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
   }

   Transform::Transform()
   {
      SetPosition(glm::vec3(0.0));
      SetRotation(glm::vec3(0.0));
      SetScale(glm::vec3(1.0));
   }

   Transform::~Transform()
   {
   }

   void Transform::SetPosition(const glm::vec3& position)
   {
      mPosition = position;
      RebuildWorldMatrix();
   }

   void Transform::SetRotation(const glm::vec3& eulerRotation)
   {
      SetOrientation(OrientationFromEuler(eulerRotation));
      RebuildWorldMatrix();
   }

   void Transform::SetScale(const glm::vec3& scale)
   {
      mScale = scale;
      RebuildWorldMatrix();
   }

   void Transform::SetOrientation(const glm::quat& quaternion)
   {
      mOrientation = quaternion;
      RebuildWorldMatrix();
   }

   void Transform::AddTranslation(const glm::vec3& translation)
   {
      mPosition += translation;
   }

   void Transform::AddRotation(const glm::vec3& eulerRotation, bool local)
   {
      glm::quat orientationDelta = OrientationFromEuler(eulerRotation);

      if (local)
         mOrientation = mOrientation * orientationDelta;
      else
         mOrientation = orientationDelta * mOrientation;

      RebuildWorldMatrix();
   }

   void Transform::AddScale(const glm::vec3& scale)
   {
      mScale += scale;
      RebuildWorldMatrix();
   }

   const glm::vec3& Transform::GetPosition() const
   {
      return mPosition;
   }

   const glm::vec3& Transform::GetScale() const
   {
      return mScale;
   }

   const glm::mat4& Transform::GetWorldMatrix() const
   {
      return mWorld;
   }

   glm::mat4 Transform::GetWorldInverseTransposeMatrix() const
   {
      return glm::inverseTranspose(mWorld);
   }

   const glm::quat& Transform::GetOrientation() const
   {
      return mOrientation;
   }

   void Transform::RebuildWorldMatrix()
   {
      glm::mat4 translation = glm::translate(glm::mat4(), mPosition);
      glm::mat4 rotation = glm::mat4_cast(mOrientation);
      glm::mat4 scale = glm::scale(glm::mat4(), mScale);
      glm::mat4 world = translation * rotation * scale;

      mWorld = world;
   }

   glm::quat Transform::OrientationFromEuler(const glm::vec3& eulerRotation)
   {
      glm::quat orientation = glm::angleAxis(eulerRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
      orientation = orientation * glm::angleAxis(eulerRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
      orientation = orientation * glm::angleAxis(eulerRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

      return orientation;
   }

}
