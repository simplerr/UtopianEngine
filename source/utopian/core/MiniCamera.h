#pragma once

#include <glm/glm.hpp>

class MiniCamera
{
public:
   MiniCamera(glm::vec3 position, glm::vec3 target, float nearPlane, float farPlane, float speed, int windowWidth, int windowHeight);

   void Update();

   glm::mat4 GetView();
   glm::mat4 GetProjection();
   glm::vec3 GetPosition() { return mPosition; };
   glm::vec3 GetTarget() { return mTarget; };
private:
   glm::vec3 mPosition;
   glm::vec3 mTarget;
   glm::vec3 mUp;
   glm::vec2 mLastCursorPos;
   glm::vec2 mWindowSize;
   float mSpeed;
   float mPitch;
   float mYaw;
   float mNear;
   float mFar;
   float mFov;
};
