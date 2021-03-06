#include "MiniCamera.h"
#include "core/Input.h"
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

MiniCamera::MiniCamera(glm::vec3 position, glm::vec3 target, int nearPlane, int farPlane, float speed, int windowWidth, int windowHeight)
   : mPosition(position), mTarget(target), mSpeed(speed)
{
   mLastCursorPos = glm::vec2(windowWidth / 2.0f, windowHeight / 2.0f);
   mWindowSize = glm::vec2(windowWidth, windowHeight);

   mUp = glm::vec3(0.0f, -1.0f, 0.0f);

   glm::vec3 ds = mTarget - mPosition;
   mYaw = atan2(ds.x, ds.z);
   mPitch = atan2(ds.y, sqrt((ds.x * ds.x) + (ds.z * ds.z)));

   mNear = nearPlane;
   mFar = farPlane;
}

void MiniCamera::Update()
{
   // Move the camera using user input
   if (Utopian::gInput().KeyDown('A'))
   {
      glm::vec3 dir = mTarget - mPosition;
      glm::vec3 right_vec = glm::cross(dir, mUp);
      right_vec = glm::normalize(right_vec) * mSpeed;

      mPosition = mPosition - right_vec;
      mTarget = mTarget - right_vec;
   }

   else if (Utopian::gInput().KeyDown('D'))
   {
      glm::vec3 dir = mTarget - mPosition;
      glm::vec3 right_vec = glm::cross(dir, mUp);
      right_vec = glm::normalize(right_vec) * mSpeed;

      mPosition = mPosition + right_vec;
      mTarget = mTarget + right_vec;
   }

   if (Utopian::gInput().KeyDown('W'))
   {
      glm::vec3 dir = mTarget - mPosition;
      dir = glm::normalize(dir) * mSpeed;
      mPosition = mPosition + dir;
      mTarget = mTarget + dir;
   }

   if (Utopian::gInput().KeyDown('S'))
   {
      glm::vec3 dir = mTarget - mPosition;
      dir = glm::normalize(dir) * mSpeed;
      mPosition = mPosition - dir;
      mTarget = mTarget - dir;
   }

   if (Utopian::gInput().KeyPressed(VK_RBUTTON))
   {
      mLastCursorPos = Utopian::gInput().GetMousePosition(); 
   }

   if (Utopian::gInput().KeyDown(VK_RBUTTON))
   {
      glm::vec2 cursorPos = Utopian::gInput().GetMousePosition();
      glm::vec2 delta = mLastCursorPos - cursorPos;

      float camera_sensitivity = 0.005f;
      mYaw -= delta.x * camera_sensitivity;
      mPitch += delta.y * camera_sensitivity;

      glm::vec3 dir = { cosf(mPitch) * sinf(mYaw),
         sinf(mPitch),
         cosf(mPitch) * cosf(mYaw)
      };

      mTarget = mPosition + dir;
   
      mLastCursorPos = cursorPos;
   }
}

glm::mat4 MiniCamera::GetView()
{
   glm::mat4 viewMatrix = glm::lookAt(GetPosition(), GetTarget(), mUp);
   return viewMatrix;
}

glm::mat4 MiniCamera::GetProjection()
{
   // Note: should be floats but it gives weird distortion effect
   glm::mat4 projectionMatrix = glm::perspective(90, (int)(mWindowSize.x / mWindowSize.y), mNear, mFar);
   return projectionMatrix;
}