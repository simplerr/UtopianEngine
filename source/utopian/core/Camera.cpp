#include "core/Camera.h"
#include "core/Window.h"
#include "vulkan/Debug.h"
#include <glm/gtc/matrix_transform.hpp>
#include "core/Input.h"
#include "core/renderer/Renderer.h"

namespace Utopian
{
   Camera::Camera(Utopian::Window* window, glm::vec3 position, float fieldOfView, float nearPlane, float farPlane)
   {
      SetPosition(position);
      this->mFov = fieldOfView;
      this->mAspectRatio = (float)window->GetWidth() / (float)window->GetHeight();
      this->mNearPlane = nearPlane;
      this->mFarPlane = farPlane;
      mUp = glm::vec3(0, 1, 0);
      mYaw = mPitch = 0.0f;
      mWindow = window;
   }

   Camera::~Camera()
   {

   }

   SharedPtr<Camera> Camera::Create(Utopian::Window* window, glm::vec3 position, float fieldOfView, float nearPlane, float farPlane)
   {
      SharedPtr<Camera> instance(new Camera(window, position, fieldOfView, nearPlane, farPlane));
      instance->Initialize();

      return instance;
   }

   void Camera::UpdateFrustum()
   {
      mFrustum.Update(GetProjection() * GetView());
   }

   void Camera::Initialize()
   {
      Utopian::Renderer::Instance().AddCamera(this);
   }

   void Camera::OnDestroyed()
   {
      Renderer::Instance().RemoveCamera(this);
   }

   void Camera::SetFov(float fov)
   {
      mFov = fov;
   }

   void Camera::SetNearPlane(float nearPlane)
   {
      mNearPlane = nearPlane;
   }

   void Camera::SetFarPlane(float farPlane)
   {
      mFarPlane = farPlane;
   }

   void Camera::SetWindow(Utopian::Window* window)
   {
      mWindow = window;
   }

   void Camera::SetAspectRatio(float aspectRatio)
   {
      mAspectRatio = aspectRatio;
   }

   Ray Camera::GetPickingRay()
   {
      // Camera/view matrix
      glm::mat4 viewMatrix = GetView();
      glm::mat4 projectionMatrix = GetProjection();

      glm::mat4 inverseView = glm::inverse(viewMatrix);
      glm::mat4 inverseProjection = glm::inverse(projectionMatrix);

      glm::vec2 cursorPos = gInput().GetMousePosition();

      float vx = (+2.0f * cursorPos.x / mWindow->GetWidth() - 1.0f);
      float vy = (-2.0f * cursorPos.y / mWindow->GetHeight() + 1.0f);

      glm::vec4 rayDir = inverseProjection * glm::vec4(-vx, vy, 1.0, 1.0);
      rayDir.z = 1.0;
      rayDir.w = 0;
      rayDir = inverseView * rayDir;
      glm::vec3 rayFinalDir = glm::normalize(glm::vec3(rayDir.x, rayDir.y, rayDir.z));

      return Utopian::Ray(GetPosition(), rayFinalDir);
   }

   glm::vec3 Camera::GetDirection()
   {
      glm::vec4 forward = glm::inverse(GetOrientation()) * glm::vec4(0, 0, 1, 1);
      glm::vec3 f = forward;
      if (f.y != -1.0f)
         int a = 1;
      f = glm::normalize(f);
      return f;
   }

   glm::vec3 Camera::GetRight()
   {
      glm::vec4 right = glm::inverse(GetOrientation()) * glm::vec4(1, 0, 0, 1);
      glm::vec3 r = right;
      r = glm::normalize(r);
      return r;
   }

   glm::vec3 Camera::GetTarget()
   {
      return GetPosition() + GetDirection();
   }

   glm::vec3 Camera::GetUp()
   {
      return mUp;
   }

   glm::vec3 Camera::GetLookAt()
   {
      return mLookAt;
   }

   glm::mat4 Camera::GetOrientation()
   {
      glm::mat4 orientation = glm::mat4();
      orientation = glm::rotate(orientation, glm::radians(mPitch), glm::vec3(1, 0, 0));      // Pitch (vertical angle)
      orientation = glm::rotate(orientation, glm::radians(mYaw), glm::vec3(0, 1, 0));        // Yaw (horizontal angle)
      orientation = glm::rotate(orientation, glm::radians(0.0f), glm::vec3(0, 0, 1));        // Yaw (horizontal angle)
      return orientation;
   }

   void Camera::AddOrientation(float yaw, float pitch)
   {
      this->mYaw += yaw;
      this->mPitch += pitch;
      CapAngles();
   }

   void Camera::SetOrientation(float yaw, float pitch)
   {
      this->mYaw = yaw;
      this->mPitch = pitch;
      CapAngles();
   }

   glm::mat4 Camera::GetView()
   {
      //mat4 viewMatrix = glm::lookAt(GetPosition(), GetTarget(), GetUp());
      //return viewMatrix;
      return GetOrientation() * glm::translate(glm::mat4(), GetPosition());
   }

   glm::mat4 Camera::GetProjection()
   {
      return glm::perspective(glm::radians(mFov), mAspectRatio, mNearPlane, mFarPlane);
   }

   glm::mat4 Camera::GetMatrix()
   {
      return GetProjection() * GetView();
   }

   void Camera::LookAt(glm::vec3 target)
   {
      glm::vec3 dir = glm::normalize(target - GetPosition());
      mPitch = glm::degrees(asinf(dir.y));
      mYaw = -glm::degrees(atan2f(dir.x, dir.z));     // Note the - signs
      mLookAt = target;
   }

   void Camera::CapAngles()
   {
      mYaw = fmodf(mYaw, 360.0f);

      if (mYaw < 0.0f)
         mYaw += 360.0f;

      if (mPitch > 85.0f)
         mPitch = 85.0;
      else if (mPitch < -85.0)
         mPitch = -85.0;
   }

   const Frustum& Camera::GetFrustum() const
   {
      return mFrustum;
   }

   float Camera::GetPitch()
   {
      return mPitch;
   }

   float Camera::GetYaw()
   {
      return mYaw;
   }

   float Camera::GetFov() const
   {
      return mFov;
   }

   float Camera::GetNearPlane() const
   {
      return mNearPlane;
   }

   float Camera::GetFarPlane() const
   {
      return mFarPlane;
   }
}  // VulkanLib namespace