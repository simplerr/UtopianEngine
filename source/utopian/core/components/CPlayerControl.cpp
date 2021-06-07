#include <core/renderer/Renderable.h>
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
#include <vulkan/ModelLoader.h>
#include "core/components/COrbit.h"
#include "core/components/CNoClip.h"
#include "core/components/CTransform.h"
#include "core/components/CCamera.h"
#include "core/components/CPlayerControl.h"
#include "core/components/CRigidBody.h"
#include "core/components/Actor.h"
#include "core/Input.h"
#include "core/physics/Physics.h"
#include "core/Object.h"
#include "core/Log.h"
#include "core/Camera.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "core/renderer/ImGuiRenderer.h"
#include "core/renderer/Renderer.h"
#include "vulkan/TextureLoader.h"

namespace Utopian
{
   CPlayerControl::CPlayerControl(Actor* parent, float maxSpeed, float jumpStrength)
      : Component(parent)
   {
      SetName("CPlayerControl");
      mMaxSpeed = maxSpeed;
      mJumpStrength = jumpStrength;

      SharedPtr<Vk::StaticModel> model = Vk::gModelLoader().LoadModel("data/models/fps_hands/fps_hands.obj");
      mViewmodel = Renderable::Create();
      mViewmodel->SetModel(model);
      mViewmodel->AddRotation(glm::vec3(glm::pi<float>(), 0.0f, 0.0f));
      mViewmodel->SetVisible(false);
      mViewmodel->RemoveRenderFlags(RENDER_FLAG_CAST_SHADOW);

      // Update the Player reference in World
      gWorld().SetPlayerActor(parent);
   }

   CPlayerControl::~CPlayerControl()
   {
   }

   void CPlayerControl::PostInit()
   {
      mTransform = GetParent()->GetComponent<CTransform>();
      mCamera = GetParent()->GetComponent<CCamera>();
      mOrbit = GetParent()->GetComponent<COrbit>();
      mNoClip = GetParent()->GetComponent<CNoClip>();
      mRigidBody = GetParent()->GetComponent<CRigidBody>();

      if (gPhysics().IsOnGround(mRigidBody))
         mMovementState = GROUND;
      else
         mMovementState = AIR;

      mFrictionRestoreValue = mRigidBody->GetFriction();

      if (mOrbit != nullptr)
         mOrbit->SetActive(false);

      const uint32_t size = 50;
      mCrosshair.texture = Vk::gTextureLoader().LoadTexture("data/textures/crosshair.png");
      mCrosshair.quad = gScreenQuadUi().AddQuad((gRenderer().GetWindowWidth() / 2) - (size / 2),
                                                (gRenderer().GetWindowHeight() / 2) - (size / 2),
                                                size, size, mCrosshair.texture.get(), 1u);
      mCrosshair.quad->visible = false;
   }

   void CPlayerControl::SetPlayMode(bool playMode)
   {
      // Todo: This should not be here, a better place is needed for managing
      // edit vs play mode
      mRigidBody->SetKinematic(playMode);
      gInput().SetVisibleCursor(playMode);
      mCrosshair.quad->visible = !playMode;
      mViewmodel->SetVisible(!playMode);

      if (ImGuiRenderer::GetMode() == UI_MODE_EDITOR)
         ImGuiRenderer::SetMode(UI_MODE_GAME);
      else
         ImGuiRenderer::SetMode(UI_MODE_EDITOR);
   }

   void CPlayerControl::Update()
   {
      // If not kinematic then the CNoClip component will control the
      // components movement instead.
      if (gInput().KeyPressed('V') || gInput().KeyPressed(VK_ESCAPE))
         SetPlayMode(!mRigidBody->IsKinematic());

      HandleMovement();
      HandleJumping();
      DrawJumpTrail();
      UpdateViewmodel();

      // No rotation
      mRigidBody->SetAngularVelocity(glm::vec3(0.0f));

      if (ImGuiRenderer::GetMode() == UI_MODE_GAME)
      {
         glm::vec2 pos = glm::vec2(gRenderer().GetWindowWidth() / 2, gRenderer().GetWindowHeight() - 200);
         ImGuiRenderer::BeginWindow("CPlayerControl UI", pos, 300.0f,
                              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
         ImGui::Text("Speed: %.2f m/s", GetCurrentSpeed());

         if (mLevelTimer.active)
            ImGui::Text("Time: %.1f", gTimer().GetElapsedTime(mLevelTimer.startTime) / 1000.0f);
         else
            ImGui::Text("Time: 0.0");

         ImGuiRenderer::EndWindow();
      }
   }

   /**
    * This function implements Quake style movement with similar airstrafing calculations.
    * References:
    * https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/game/shared/gamemovement.cpp
    * https://web.archive.org/web/20190428135531/http://www.funender.com/quake/articles/strafing_theory.html
    * https://www.jwchong.com/hl/strafing.html
    * http://adrianb.io/2015/02/14/bunnyhop.html
    * https://steamcommunity.com/sharedfiles/filedetails/?id=184184420
    */
   void CPlayerControl::HandleMovement()
   {
      glm::vec3 wishVel = CalculateWishVelocity();
      glm::vec3 wishDir = glm::normalize(wishVel);
      float wishSpeed = glm::length(wishVel);

      if (wishSpeed > mMaxSpeed)
         wishSpeed = mMaxSpeed;

      if (wishVel != glm::vec3(0.0f))
      {
         glm::vec3 acceleration = glm::vec3(0.0f);

         if (gPhysics().IsOnGround(mRigidBody))
         {
            acceleration = Accelerate(wishDir, wishSpeed, mGroundAcceleration, false);
         }
         else
         {
            acceleration = Accelerate(wishDir, wishSpeed, mAirAcceleration, true);
         }

         if (acceleration != glm::vec3(0.0f))
         {
            glm::vec3 impulse = mRigidBody->GetMass() * acceleration;
            mRigidBody->ApplyCentralImpulse(impulse);
         }
      }
   }

   glm::vec3 CPlayerControl::CalculateWishVelocity()
   {
      glm::vec3 forward = glm::normalize(glm::vec3(mCamera->GetDirection().x, 0.0f, mCamera->GetDirection().z));
      glm::vec3 right = glm::normalize(glm::vec3(mCamera->GetRight().x, 0.0f, mCamera->GetRight().z));

      float forwardMove = 0.0f;
      float sideMove = 0.0f;

      if (gInput().KeyDown('A'))
         sideMove = mMaxSpeed;
      else if (gInput().KeyDown('D'))
         sideMove = -mMaxSpeed;

      if (gInput().KeyDown('W'))
         forwardMove = mMaxSpeed;
      else if (gInput().KeyDown('S'))
         forwardMove = -mMaxSpeed;

      glm::vec3 wishVel = forward * forwardMove + right * sideMove;
      wishVel.y = 0.0f;

      return wishVel;
   }

   glm::vec3 CPlayerControl::Accelerate(glm::vec3 wishDir, float wishSpeed, float airAccelerate, bool inAir)
   {
      float wishSpd = wishSpeed;

      // Cap speed
      if (inAir)
      {
         if (wishSpd > mAirSpeedCap)
            wishSpd = mAirSpeedCap;
      }

      // Project current velocity on wishDir vector
      float projVel = glm::dot(mRigidBody->GetVelocity(), wishDir);

      // See how much to add
      float addSpeed = wishSpd - projVel;

      // If not adding any, done.
      if (addSpeed <= 0)
         return glm::vec3(0.0f);

      // Determine acceleration speed after acceleration
      float accelSpeed = airAccelerate * wishSpeed; // * gpGlobals->frametime * player->m_surfaceFriction;

      // Cap speed
      if (accelSpeed > addSpeed)
         accelSpeed = addSpeed;

      glm::vec3 acceleration = accelSpeed * wishDir;

      return acceleration;
   }

   /**
    * This function implements a state machine for jumping to change
    * the friction when in the air to allow bhopping.
    * Note: A temporary hack, does not mirror Quake implementation.
    */
   void CPlayerControl::HandleJumping()
   {
      bool onGround = gPhysics().IsOnGround(mRigidBody);

      // Jump with space and mwheelup
      if ((gInput().KeyPressed(VK_SPACE) || gInput().MouseDz() > 0.0f) &&
          ((mMovementState == REDUCED_FRICTION) ||
           (mMovementState == GROUND)))
      {
         if (mMovementState == GROUND)
            mFrictionRestoreValue = mRigidBody->GetFriction();

         mRigidBody->ApplyCentralImpulse(mRigidBody->GetMass() * glm::vec3(0.0f, mJumpStrength, 0.0f));
         mRigidBody->SetFriction(0.0f);

         // Not in the air immediately
         mMovementState = POST_JUMP;

         mJumpPosition = GetParent()->GetTransform().GetPosition();
         mJumpTrailPoints.clear();
      }

      // Now the rigid body is fully in the air
      if (mMovementState == POST_JUMP && !onGround)
      {
         mMovementState = AIR;
      }

      // Add points to the jump trail
#ifdef ENABLE_JUMP_TRAILS
      if (mMovementState == AIR)
      {
         static Timestamp lastTimestamp = gTimer().GetTimestamp();

         const float trailFrequency = 0.1f;
         if (gTimer().GetElapsedTime(lastTimestamp) > trailFrequency)
         {
            TrailingPoint point = { GetParent()->GetTransform().GetPosition(), Im3d::Color_Yellow };
            glm::vec3 vel = mRigidBody->GetVelocity();
            vel.y = 0.0f;
            point.color = glm::length(vel) > 4.0 ? Im3d::Color_Green : point.color;

            mJumpTrailPoints.push_back(point);
            lastTimestamp = gTimer().GetTimestamp();
         }
      }
#endif

      // Keep zero friction a short time after landing to allow bhopping
      if (mMovementState == AIR && onGround)
      {
         mLandingTimestamp = gTimer().GetTimestamp();
         mMovementState = REDUCED_FRICTION;

         glm::vec3 jumpDelta = GetParent()->GetTransform().GetPosition() - mJumpPosition;
         jumpDelta.y = 0.0f;
         float jumpDistance = glm::length(jumpDelta);
         UTO_LOG("Jump distance: " + std::to_string(jumpDistance));
      }

      // Restore friction
      if (mMovementState == REDUCED_FRICTION && gTimer().GetElapsedTime(mLandingTimestamp) > mReducedFrictionTime)
      {
         mMovementState = GROUND;
         mRigidBody->SetFriction(mFrictionRestoreValue);
      }
   }

   void CPlayerControl::DrawJumpTrail()
   {
      for (const auto& point : mJumpTrailPoints)
      {
         Im3d::DrawPoint(point.pos, 20.0f, point.color);
      }
   }

   void CPlayerControl::UpdateViewmodel()
   {
      // Position
      glm::vec3 cameraDir = gRenderer().GetMainCamera()->GetDirection();
      glm::vec3 handPos = gRenderer().GetMainCamera()->GetPosition();
      handPos.y += handY;
      handPos += cameraDir * handZ;
      mViewmodel->SetPosition(handPos);

      // Rotation
      float rotationY = glm::atan(cameraDir.z, cameraDir.x);
      mViewmodel->SetRotation(glm::vec3(glm::pi<float>(),
                                        rotationY - (glm::pi<float>() / 2.0f),
                                        0.0f));
   }

   void CPlayerControl::StartLevelTimer()
   {
      mLevelTimer.startTime = gTimer().GetTimestamp();
      mLevelTimer.active = true;
   }

   void CPlayerControl::StopLevelTimer()
   {
      if (mLevelTimer.active)
      {
         mLevelTimer.active = false;
         float elapsedTime = (float)gTimer().GetElapsedTime(mLevelTimer.startTime) / 1000.0f;
         UTO_LOG("Finished the level in " + std::to_string(elapsedTime) + " seconds!");
      }
   }

   LuaPlus::LuaObject CPlayerControl::GetLuaObject()
   {
      LuaPlus::LuaObject luaObject;
      luaObject.AssignNewTable(gLuaManager().GetLuaState());

      luaObject.SetNumber("maxSpeed", mMaxSpeed);
      luaObject.SetNumber("jumpStrength", mJumpStrength);

      return luaObject;
   }

   void CPlayerControl::SetMaxSpeed(float maxSpeed)
   {
      mMaxSpeed = maxSpeed;
   }

   void CPlayerControl::SetJumpStrength(float jumpStrength)
   {
      mJumpStrength = jumpStrength;
   }

   void CPlayerControl::SetAirAccelerate(float airAccelerate)
   {
      mAirAcceleration = airAccelerate;
   }

   void CPlayerControl::SetAirSpeedCap(float airSpeedCap)
   {
      mAirSpeedCap = airSpeedCap;
   }

   float CPlayerControl::GetMaxSpeed() const
   {
      return mMaxSpeed;
   }

   float CPlayerControl::GetJumpStrength() const
   {
      return mJumpStrength;
   }

   float CPlayerControl::GetAirAccelerate() const
   {
      return mAirAcceleration;
   }

   float CPlayerControl::GetAirSpeedCap() const
   {
      return mAirSpeedCap;
   }

   MovementState CPlayerControl::GetMovementState() const
   {
      return mMovementState;
   }

   float CPlayerControl::GetCurrentSpeed() const
   {
      glm::vec3 vel = mRigidBody->GetVelocity();
      vel.y = 0.0f;

      return glm::length(vel);
   }
}