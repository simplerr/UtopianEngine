#include "core/Physics.h"
#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"
#include "BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"

namespace Utopian
{
	Physics& gPhysics()
	{
		return Physics::Instance();
	}

	Physics::Physics()
	{
		// Note: Experienced heap corruption when BT_USE_DOUBLE_PRECISION was not added to the preprocessor
		mBroadphase = new btDbvtBroadphase();
		mCollisionConfiguration = new btDefaultCollisionConfiguration();
		mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
		mConstraintSolver = new btSequentialImpulseConstraintSolver();
		mDynamicsWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mConstraintSolver, mCollisionConfiguration);

		mDynamicsWorld->setGravity(btVector3(mGravity.x, mGravity.y, mGravity.z));
	}

	Physics::~Physics()
	{
		delete mDynamicsWorld;
		delete mConstraintSolver;
		delete mDispatcher;
		delete mCollisionConfiguration;
		delete mBroadphase;
	}

	void Physics::Update()
	{
		mDynamicsWorld->stepSimulation(1.f / 60.f, 10);
	}

	void Physics::Draw()
	{

	}

	btDiscreteDynamicsWorld* Physics::GetDynamicsWorld() const
	{
		return mDynamicsWorld;
	}
}