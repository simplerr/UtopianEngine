#include "core/components/COrbit.h"
#include "core/components/CNoClip.h"
#include "core/components/CTransform.h"
#include "core/components/CCamera.h"
#include "core/components/CPlayerControl.h"
#include "core/components/CRigidBody.h"
#include "core/components/Actor.h"
#include "core/Input.h"

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
		if (gInput().KeyPressed('F'))
		{
			mRigidBody->SetKinematic(!mRigidBody->IsKinematic());
		}

		glm::vec3 velocity = glm::vec3(0.0f);
		glm::vec3 direction = mCamera->GetDirection();
		direction = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));

		if (gInput().KeyDown('A'))
		{
			velocity += mSpeed * mCamera->GetRight();
		}
		else if (gInput().KeyDown('D'))
		{
			velocity -= mSpeed * mCamera->GetRight();
		}

		if (gInput().KeyDown('W'))
		{
			velocity += mSpeed * direction;
		}
		else if (gInput().KeyDown('S'))
		{
			velocity -= mSpeed * direction;
		}

		// Movement in XZ
		if (velocity != glm::vec3(0.0f))
		{
			glm::vec3 currentVelocity = mRigidBody->GetVelocity();
			currentVelocity = glm::vec3(currentVelocity.x, 0.0f, currentVelocity.z);
			glm::vec3 delta = velocity - currentVelocity;
			glm::vec3 impulse = mRigidBody->GetMass() * delta;
			mRigidBody->ApplyCentralImpulse(impulse);
		}

		// Jump
		if (gInput().KeyPressed(VK_SPACE))
		{
			mRigidBody->ApplyCentralImpulse(mRigidBody->GetMass() * glm::vec3(0.0f, mJumpStrength, 0.0f));
		}

		// No rotation
		mRigidBody->SetAngularVelocity(glm::vec3(0.0f));
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

	float CPlayerControl::GetSpeed() const
	{
		return mSpeed;
	}

	float CPlayerControl::GetJumpStrength() const
	{
		return mJumpStrength;
	}
}