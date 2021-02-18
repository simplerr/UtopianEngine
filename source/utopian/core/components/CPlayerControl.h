#pragma once
#include <glm/glm.hpp>
#include "core/components/CRigidBody.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "core/renderer/Renderable.h"
#include "core/components/Component.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"
#include "utility/Timer.h"
#include "core/LuaManager.h"
#include "im3d/im3d.h"

namespace Utopian
{
	class Actor;
	class CCamera;
	class CTransform;
	class COrbit;
	class CNoClip;
	class CRigidBody;

	enum MovementState
	{
		REDUCED_FRICTION, 	// A short time after landing the player friction is reduced to allow bhopping
		GROUND,
		AIR,
		POST_JUMP,			// The player will still be touching ground a short time after applying the jump impulse
	};

	class CPlayerControl : public Component
	{
	public:
		CPlayerControl(Actor* parent, float maxSpeed, float jumpStrength);
		~CPlayerControl();

		void Update() override;
		void PostInit() override;

		void StartLevelTimer();
		void StopLevelTimer();

		LuaPlus::LuaObject GetLuaObject() override;
		
		void SetMaxSpeed(float maxSpeed);
		void SetJumpStrength(float jumpStrength);
		void SetAirAccelerate(float airAccelerate);
		void SetAirSpeedCap(float airSpeedCap);
		void SetPlayMode(bool playMode);

		float GetMaxSpeed() const;
		float GetJumpStrength() const;
		float GetAirAccelerate() const;
		float GetAirSpeedCap() const;
		MovementState GetMovementState() const;
		float GetCurrentSpeed() const;

		// Type identification
		static uint32_t GetStaticType() {
			return Component::ComponentType::PLAYER_CONTROL;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		void HandleMovement();
		void HandleJumping();
		glm::vec3 CalculateWishVelocity();
		glm::vec3 Accelerate(glm::vec3 wishDir, float wishSpeed, float airAccelerate, bool inAir);
		void DrawJumpTrail();
		void UpdateViewmodel();
	private:
		CCamera* mCamera; // For convenience
		CNoClip* mNoClip;
		COrbit* mOrbit;
		CTransform* mTransform;
		CRigidBody* mRigidBody;
		float mMaxSpeed = 3.0f;
		float mJumpStrength = 5.0f;

		float mGroundAcceleration = 0.1f;
		float mAirAcceleration = 0.1f;
		float mAirSpeedCap = 0.3f;

		// Jumping with reduced friction
		Timestamp mLandingTimestamp;
		MovementState mMovementState;
		float mReducedFrictionTime = 50.0f;
		float mFrictionRestoreValue;

		// Statistics
		glm::vec3 mJumpPosition;
		struct TrailingPoint {
			glm::vec3 pos;
			Im3d::Color color;
		};

		std::vector<TrailingPoint> mJumpTrailPoints;

		struct Crosshair {
			SharedPtr<Vk::Texture> texture;
			SharedPtr<ScreenQuad> quad;
		} mCrosshair;

		SharedPtr<Renderable> mViewmodel;

		struct LevelTimer {
			Timestamp startTime;
			bool active = false;
		} mLevelTimer;

	public:
		float handY = -.154f;
		float handZ = 0.066f;
	};
}
