#include "core/components/CRigidBody.h"
#include "core/components/CTransform.h"
#include "core/components/Actor.h"
#include "core/Physics.h"
#include "core/BulletHelpers.h"
#include "imgui/imgui.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "btBulletDynamicsCommon.h"

namespace Utopian
{
	MotionState::MotionState(CRigidBody* rigidBody)
	{
		mRigidBody = rigidBody;
	}

	void MotionState::getWorldTransform(btTransform& worldTrans) const
	{
		glm::vec3 position = mRigidBody->GetTransform().GetPosition();
		glm::vec3 rotation = mRigidBody->GetTransform().GetRotation();

		worldTrans.setOrigin(ToBulletVec3(position));
		//worldTrans.setBasis(ToBtVec3(rotation));
	}

	void MotionState::setWorldTransform(const btTransform& worldTrans)
	{
		//Quaternion newWorldRot = ToQuaternion(worldTrans.getRotation());
		glm::vec3 position = ToVec3(worldTrans.getOrigin());

		mRigidBody->SetPosition(position);
		// Todo:
		//mRigidBody->SetRotation(rotation);
	}

	CRigidBody::CRigidBody(Actor* parent)
		: Component(parent)
	{
		SetName("CRigidBody");

		mMass = 1.0f;
		mFriction = 0.5f;
		mFrictionRolling = 0.0f;
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

		MotionState* motionState = new MotionState(this);

		btVector3 localInertia(0, 0, 0);
		mCollisionShape = new btSphereShape(btScalar(1.0f));
		mCollisionShape->calculateLocalInertia(mMass, localInertia);

		btRigidBody::btRigidBodyConstructionInfo constructionInfo(mMass, motionState, mCollisionShape, localInertia);
		constructionInfo.m_mass = mMass;
		constructionInfo.m_friction = mFriction;
		constructionInfo.m_rollingFriction = mFrictionRolling;
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

	void CRigidBody::SetPosition(const glm::vec3& position)
	{
		mTransform->SetPosition(position);
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
}