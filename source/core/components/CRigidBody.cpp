#include "core/components/CRigidBody.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/Actor.h"
#include "core/Physics.h"
#include "core/BulletHelpers.h"
#include "imgui/imgui.h"
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
		// Todo: This needs to be called by the application in some way
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

		btVector3 localInertia(0, 0, 0);
		BoundingBox aabb = mRenderable->GetBoundingBox();
		mCollisionShape = new btBoxShape(btVector3(aabb.GetWidth() / 2.0f, aabb.GetHeight() / 2.0f, aabb.GetDepth() / 2.0f));
		mCollisionShape->calculateLocalInertia(mMass, localInertia);

		btRigidBody::btRigidBodyConstructionInfo constructionInfo(mMass, motionState, mCollisionShape, localInertia);
		constructionInfo.m_mass = mMass;
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
			// Todo: Correct?
			//mRigidBody->forceActivationState(ISLAND_SLEEPING);
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
		mMass = mass;

		// Need to recalculate inertia when modifying mass
		btVector3 localInertia(0, 0, 0);
		BoundingBox aabb = mRenderable->GetBoundingBox();
		btBoxShape collisionShape = btBoxShape(btVector3(aabb.GetWidth() / 2.0f, aabb.GetHeight() / 2.0f, aabb.GetDepth() / 2.0f));
		mCollisionShape->calculateLocalInertia(mMass, localInertia);

		mRigidBody->setMassProps(mass, localInertia);
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