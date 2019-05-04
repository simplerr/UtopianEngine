#include "core/physics/Physics.h"
#include "core/physics/PhysicsDebugDraw.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include <limits>

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

		mDebugDrawer = new PhysicsDebugDraw();
		mDynamicsWorld->setDebugDrawer(mDebugDrawer);

		mHeightmapCopy = nullptr;
		mTerrainBody = nullptr;

		mEnabled = true;
		mDebugDrawEnabled = true;

		// Add ground shape for experimentation
		//AddGroundShape();
	}

	Physics::~Physics()
	{
		delete mDynamicsWorld;
		delete mConstraintSolver;
		delete mDispatcher;
		delete mCollisionConfiguration;
		delete mBroadphase;
		delete mDebugDrawer;
		delete mHeightmapCopy;
		delete mTerrainBody;
	}

	void Physics::Update()
	{
		// Update elapsed time
		auto now = std::chrono::high_resolution_clock::now();
		double deltaTime = std::chrono::duration<double, std::milli>(now - mLastFrameTime).count();
		deltaTime /= 1000.0f; // To seconds
		mLastFrameTime = now;

		if (IsEnabled())
		{
			mDynamicsWorld->stepSimulation(deltaTime, 5);
		}

		if (IsDebugDrawEnabled())
		{
			mDynamicsWorld->debugDrawWorld();
		}
	}

	void Physics::EnableSimulation(bool enable)
	{
		mEnabled = enable;
	}

	void Physics::EnableDebugDraw(bool enable)
	{
		mDebugDrawEnabled = enable;
	}

	void Physics::AddGroundShape()
	{
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

	void Physics::SetHeightmap(const float* heightmap, const uint32_t size, float scale, float terrainSize)
	{
		// Bullet uses doubles internally
		if (mHeightmapCopy != nullptr)
			delete mHeightmapCopy;

		mHeightmapCopy = new double[size * size];

		// Bullet expects the UV coordinates to be flipped.
		// Terrain::GeneratePatches() calculates them as Pos(0, 0) = Tex(0, 0) while
		// Bullets expects Pos(0, 0) = Tex(1, 1)
		for (uint32_t i = 0; i < size * size; i++)
		{
			uint32_t x = i % size;
			uint32_t y = i / size;
			uint32_t index = ((size - 1) - x) + (((size - 1) - y) * size);
			mHeightmapCopy[i] = -heightmap[index]; // Note: The negative sign
		}

		double minHeight = std::numeric_limits<double>::max();
		double maxHeight = std::numeric_limits<double>::min();

		for (uint32_t i = 0; i < size * size; i++)
		{
			if (mHeightmapCopy[i] < minHeight)
				minHeight = mHeightmapCopy[i];
			else if (mHeightmapCopy[i] > maxHeight)
				maxHeight = mHeightmapCopy[i];
		}

		btHeightfieldTerrainShape* terrainShape = new btHeightfieldTerrainShape(size, size, mHeightmapCopy, scale, minHeight, maxHeight, 1, PHY_FLOAT, false);

		float gridScaling = terrainSize / size;
		btVector3 localScaling = btVector3(gridScaling, scale, gridScaling);
		terrainShape->setLocalScaling(localScaling);

		float minHeightScaled = minHeight * scale;
		float maxHeightScaled = maxHeight * scale;

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -(maxHeightScaled - minHeightScaled) / 2.0f, 0));

		btScalar mass(0.0f);
		btVector3 localInertia(0, 0, 0);
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, terrainShape, localInertia);
		rbInfo.m_restitution = 0.0f;
		rbInfo.m_friction = 1.0f;

		if (mTerrainBody != nullptr)
		{
			mDynamicsWorld->removeRigidBody(mTerrainBody);
			delete mTerrainBody;
		}

		btRigidBody* mTerrainBody = new btRigidBody(rbInfo);
		mDynamicsWorld->addRigidBody(mTerrainBody);
	}

	void Physics::Draw()
	{

	}

	btDiscreteDynamicsWorld* Physics::GetDynamicsWorld() const
	{
		return mDynamicsWorld;
	}

	bool Physics::IsEnabled() const
	{
		return mEnabled;
	}
	
	bool Physics::IsDebugDrawEnabled() const
	{
		return mDebugDrawEnabled;
	}
}