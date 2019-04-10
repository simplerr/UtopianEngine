#include "core/components/CRigidBody.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/Actor.h"
#include "core/Physics.h"
#include "core/BulletHelpers.h"
#include "imgui/imgui.h"
#include "im3d/im3d.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "btBulletDynamicsCommon.h"
#include <glm/gtc/quaternion.hpp>

namespace Utopian
{
	MotionState::MotionState(CRigidBody* rigidBody)
	{
		mRigidBody = rigidBody;
	}

	void MotionState::getWorldTransform(btTransform& worldTrans) const
	{
		glm::vec3 position = mRigidBody->GetTransform().GetPosition();
		glm::quat orientation = mRigidBody->GetTransform().GetOrientation();

		worldTrans.setOrigin(ToBulletVec3(position));
		worldTrans.setRotation(ToBulletQuaternion(orientation));
	}

	void MotionState::setWorldTransform(const btTransform& worldTrans)
	{
		glm::vec3 position = ToVec3(worldTrans.getOrigin());
		glm::quat quaternion = ToQuaternion(worldTrans.getRotation());

		mRigidBody->SetPosition(position);
		mRigidBody->SetQuaternion(quaternion);
	}

	CRigidBody::CRigidBody(Actor* parent)
		: Component(parent)
	{
		SetName("CRigidBody");

		mMass = 1.0f;
		mFriction = 0.5f;
		mRollingFriction = 0.0f;
		mRestitution = 0.0f;
		mIsKinematic = false;
		mRigidBody = nullptr;
	}

	CRigidBody::~CRigidBody()
	{
	}

	void CRigidBody::Update()
	{
		glm::vec3 position = GetTransform().GetPosition();
		Im3d::DrawAlignedBox(position, position + glm::vec3(200.0f));
	}

	void CRigidBody::OnCreated()
	{
		
	}

	void CRigidBody::OnDestroyed()
	{
		RemoveFromWorld();
	}

	void CRigidBody::PostInit()
	{
		mTransform = GetParent()->GetComponent<CTransform>();
		mRenderable = GetParent()->GetComponent<CRenderable>();

		AddToWorld();
	}

	void CRigidBody::AddToWorld()
	{
		RemoveFromWorld();

		MotionState* motionState = new MotionState(this);

		// Zero mass when kinematic
		float mass = mMass;
		if (IsKinematic())
			mass = 0.0f;

		btVector3 localInertia(0, 0, 0);
		BoundingBox aabb = mRenderable->GetBoundingBox();
		mCollisionShape = new btBoxShape(btVector3(aabb.GetWidth() / 2.0f, aabb.GetHeight() / 2.0f, aabb.GetDepth() / 2.0f));
		mCollisionShape->calculateLocalInertia(mass, localInertia);

		btRigidBody::btRigidBodyConstructionInfo constructionInfo(mass, motionState, mCollisionShape, localInertia);
		constructionInfo.m_mass = mass;
		constructionInfo.m_friction = mFriction;
		constructionInfo.m_rollingFriction = mRollingFriction;
		constructionInfo.m_restitution = mRestitution;

		mRigidBody = new btRigidBody(constructionInfo);
		mRigidBody->setUserPointer(this);
		mRigidBody->activate();

		// Initial position
		btTransform& worldTransform = mRigidBody->getWorldTransform();
		worldTransform.setOrigin(ToBulletVec3(GetTransform().GetPosition()));

		// Add to physics simulation
		gPhysics().GetDynamicsWorld()->addRigidBody(mRigidBody);

		UpdateKinematicFlag();
	}

	void CRigidBody::RemoveFromWorld()
	{
		if (mRigidBody == nullptr)
			return;

		gPhysics().GetDynamicsWorld()->removeRigidBody(mRigidBody);

		delete mRigidBody->getMotionState();
		delete mRigidBody;

		mRigidBody = nullptr;
	}

	void CRigidBody::UpdateKinematicFlag()
	{
		uint32_t flags = mRigidBody->getCollisionFlags();

		if (IsKinematic())
		{
			flags |= btCollisionObject::CF_KINEMATIC_OBJECT;

			mRigidBody->forceActivationState(DISABLE_DEACTIVATION);
		}
		else
		{
			flags &= btCollisionObject::CF_KINEMATIC_OBJECT;
		}

		mRigidBody->setCollisionFlags(flags);
	}

	bool CRigidBody::IsActive() const
	{
		return mRigidBody->isActive();
	}

	void CRigidBody::SetPosition(const glm::vec3& position)
	{
		mTransform->SetPosition(position);
	}

	void CRigidBody::SetRotation(const glm::vec3& rotation)
	{
		mTransform->SetRotation(rotation);
	}

	void CRigidBody::SetQuaternion(const glm::quat& quaternion)
	{
		mTransform->SetOrientation(quaternion);
	}

	const Utopian::Transform& CRigidBody::GetTransform() const
	{
		return mTransform->GetTransform();
	}

	LuaPlus::LuaObject CRigidBody::GetLuaObject()
	{
		LuaPlus::LuaObject luaObject;
		luaObject.AssignNewTable(gLuaManager().GetLuaState());

		luaObject.SetNumber("empty", 0.0f);

		return luaObject;
	}

	float CRigidBody::GetMass() const
	{
		return mMass;
	}

	float CRigidBody::GetFriction() const
	{
		return mFriction;
	}

	float CRigidBody::GetRollingFriction() const
	{
		return mRollingFriction;
	}

	float CRigidBody::GetRestitution() const
	{
		return mRestitution;
	}

	bool CRigidBody::IsKinematic() const
	{
		return mIsKinematic;
	}

	void CRigidBody::SetMass(float mass)
	{
		if (mMass == mass)
			return;

		mMass = mass;

		// If the body is kinematic the mass needs to be 0
		// The newly assigned mass will become active when the body becomes dynamic again
		if (IsKinematic())
			return;

		AddToWorld();
	}

	void CRigidBody::SetFriction(float friction)
	{
		mFriction = friction;
		mRigidBody->setFriction(friction);
	}

	void CRigidBody::SetRollingFriction(float rollingFriction)
	{
		mRollingFriction = rollingFriction;
		mRigidBody->setRollingFriction(rollingFriction);
	}

	void CRigidBody::SetRestitution(float restitution)
	{
		mRestitution = restitution;
		mRigidBody->setRestitution(restitution);
	}

	void CRigidBody::SetKinematic(bool isKinematic)
	{
		if (mIsKinematic == isKinematic)
			return;

		mIsKinematic = isKinematic;

		AddToWorld();
	}
}