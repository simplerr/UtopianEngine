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
		glm::vec3 rotation = mRigidBody->GetTransform().GetRotation();
		glm::quat quaternion = glm::quat(glm::radians(rotation));

		quaternion = quaternion * quaternion;

		worldTrans.setOrigin(ToBulletVec3(position));
		worldTrans.setRotation(ToBulletQuaternion(quaternion));
	}

	void MotionState::setWorldTransform(const btTransform& worldTrans)
	{
		glm::vec3 position = ToVec3(worldTrans.getOrigin());
		glm::quat quaternion = ToQuaternion(worldTrans.getRotation());
		glm::vec3 rotation = glm::eulerAngles(quaternion);
		//glm::mat4 rotationMatrix = glm::mat4_cast(quaternion);

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
	}

	void CRigidBody::PostInit()
	{
		mTransform = GetParent()->GetComponent<CTransform>();
		mRenderable = GetParent()->GetComponent<CRenderable>();

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
	}

	void CRigidBody::Activate()
	{
		mRigidBody->activate();
	}

	void CRigidBody::Deactivate()
	{
		// Seems like this will activate the rigid body again on collision
		mRigidBody->setActivationState(WANTS_DEACTIVATION);
	}
	
	bool CRigidBody::IsActiveted()
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
		mTransform->SetQuaternion(quaternion);
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
}