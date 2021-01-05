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
#include <glm/geometric.hpp>

namespace Utopian
{
	CPlayerControl::CPlayerControl(Actor* parent)
		: Component(parent)
	{
		SetName("CPlayerControl");
	}

	CPlayerControl::~CPlayerControl()
	{
	}

	void CPlayerControl::Update()
	{
		// If not kinematic then the CNoClip component will control the 
		// components movement instead.
		if (gInput().KeyPressed('V'))
		{
			mRigidBody->SetKinematic(!mRigidBody->IsKinematic());
			gInput().SetVisibleCursor(mRigidBody->IsKinematic());
		}

		HandleMovement();

		// Jump
		if (gInput().KeyPressed(VK_SPACE) && gPhysics().IsOnGround(mRigidBody))
		{
			mRigidBody->ApplyCentralImpulse(mRigidBody->GetMass() * glm::vec3(0.0f, mJumpStrength, 0.0f));
		}

		// No rotation
		mRigidBody->SetAngularVelocity(glm::vec3(0.0f));
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

		const float maxSpeed = 3.0f;
		if (wishSpeed > maxSpeed)
			wishSpeed = maxSpeed;

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
			sideMove = mSpeed;
		else if (gInput().KeyDown('D'))
			sideMove = -mSpeed;

		if (gInput().KeyDown('W'))
			forwardMove = mSpeed;
		else if (gInput().KeyDown('S'))
			forwardMove = -mSpeed;

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

		glm::vec3 xzVel = mRigidBody->GetVelocity();
		xzVel.y = 0.0f;
		UTO_LOG("realVel" + std::to_string(glm::length(xzVel)));
		UTO_LOG("projVel" + std::to_string(projVel));

		glm::vec3 acceleration = accelSpeed * wishDir;

		return acceleration;
	}

	void CPlayerControl::PostInit()
	{
		mTransform = GetParent()->GetComponent<CTransform>();
		mCamera = GetParent()->GetComponent<CCamera>();
		mOrbit = GetParent()->GetComponent<COrbit>();
		mNoClip = GetParent()->GetComponent<CNoClip>();
		mRigidBody = GetParent()->GetComponent<CRigidBody>();

		if (mOrbit != nullptr)
			mOrbit->SetActive(false);
	}

	LuaPlus::LuaObject CPlayerControl::GetLuaObject()
	{
		LuaPlus::LuaObject luaObject;
		luaObject.AssignNewTable(gLuaManager().GetLuaState());

		luaObject.SetNumber("speed", mSpeed);
		luaObject.SetNumber("jumpStrength", mJumpStrength);

		return luaObject;
	}

	void CPlayerControl::SetSpeed(float speed)
	{
		mSpeed = speed;
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

	float CPlayerControl::GetSpeed() const
	{
		return mSpeed;
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
}