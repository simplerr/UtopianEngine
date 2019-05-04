#pragma once
#include "utility/Module.h"
#include <glm/glm.hpp>
#include <chrono>

class btBroadphaseInterface;
class btCollisionDispatcher;
class btConstraintSolver;
class btDefaultCollisionConfiguration;
class btDiscreteDynamicsWorld;
class btRigidBody;

namespace Utopian
{
	class PhysicsDebugDraw;

	class Physics : public Module<Physics>
	{
	public:
		Physics();
		~Physics();

		void Update();
		void Draw();
		void EnableSimulation(bool enable);
		void EnableDebugDraw(bool enable);

		void SetHeightmap(const float* heightmap, const uint32_t size, float scale, float terrainSize);

		bool IsEnabled() const;
		bool IsDebugDrawEnabled() const;

		btDiscreteDynamicsWorld* GetDynamicsWorld() const;

	private:
		void AddGroundShape();

	private:
		btBroadphaseInterface* mBroadphase;
		btCollisionDispatcher* mDispatcher;
		btConstraintSolver* mConstraintSolver;
		btDefaultCollisionConfiguration* mCollisionConfiguration;
		btDiscreteDynamicsWorld* mDynamicsWorld;
		PhysicsDebugDraw* mDebugDrawer;

		btRigidBody* mTerrainBody;

		const glm::vec3 mGravity = glm::vec3(0, -982., 0);
		bool mEnabled;
		bool mDebugDrawEnabled;
		double* mHeightmapCopy;

		// Note: This should be handled by the Timer component.
		// But since the calls to ImGui::NewFrame() and ImGui::Render() currently are not 
		// called at the same frequency as UIOverlay::Update.
		std::chrono::high_resolution_clock::time_point mLastFrameTime;
	};

	Physics& gPhysics();
}
