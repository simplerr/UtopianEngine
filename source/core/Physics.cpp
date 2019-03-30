#include "core/Physics.h"
#include "btBulletDynamicsCommon.h"

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

		// Add ground shape for experimentation
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(5000.), btScalar(500.), btScalar(5000.)));

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -500, 0));

		btScalar mass(0.);

		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		rbInfo.m_restitution = 0.0f;
		rbInfo.m_friction = 1.0f;

		btRigidBody* body = new btRigidBody(rbInfo);

		mDynamicsWorld->addRigidBody(body);
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
		// Update elapsed time
		auto now = std::chrono::high_resolution_clock::now();
		double deltaTime = std::chrono::duration<double, std::milli>(now - mLastFrameTime).count();
		deltaTime /= 1000.0f; // To seconds
		mLastFrameTime = now;

		mDynamicsWorld->stepSimulation(deltaTime, 5);
	}

	void Physics::Draw()
	{

	}

	btDiscreteDynamicsWorld* Physics::GetDynamicsWorld() const
	{
		return mDynamicsWorld;
	}
}