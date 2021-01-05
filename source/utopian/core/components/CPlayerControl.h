#pragma once
#include <core/components/CRigidBody.h>
#include <glm/glm.hpp>
#include "core/components/Component.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"
#include "core/LuaManager.h"

namespace Utopian
{
	class Actor;
	class CCamera;
	class CTransform;
	class COrbit;
	class CNoClip;
	class CRigidBody;

	class CPlayerControl : public Component
	{
	public:
		CPlayerControl(Actor* parent);
		~CPlayerControl();

		void Update() override;
		void PostInit() override;

		LuaPlus::LuaObject GetLuaObject() override;
		
		void SetSpeed(float speed);
		void SetJumpStrength(float jumpStrength);
		void SetAirAccelerate(float airAccelerate);
		void SetAirSpeedCap(float airSpeedCap);

		float GetSpeed() const;
		float GetJumpStrength() const;
		float GetAirAccelerate() const;
		float GetAirSpeedCap() const;

		// Type identification
		static uint32_t GetStaticType() {
			return Component::ComponentType::PLAYER_CONTROL;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		void HandleMovement();
		glm::vec3 CalculateWishVelocity();
		glm::vec3 Accelerate(glm::vec3 wishDir, float wishSpeed, float airAccelerate, bool inAir);
	private:
		CCamera* mCamera; // For convenience
		CNoClip* mNoClip;
		COrbit* mOrbit;
		CTransform* mTransform;
		CRigidBody* mRigidBody;
		float mSpeed = 3.0f;
		float mJumpStrength = 5.0f;

		float mGroundAcceleration = 0.1f;
		float mAirAcceleration = 0.1f;
		float mAirSpeedCap = 0.3f;
	};
}
