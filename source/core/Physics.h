#pragma once
#include "utility/Module.h"
#include <glm/glm.hpp>
#include <chrono>

class btBroadphaseInterface;
class btCollisionDispatcher;
class btConstraintSolver;
class btDefaultCollisionConfiguration;
class btDiscreteDynamicsWorld;

namespace Utopian
{
	class Physics : public Module<Physics>
	{
	public:
		Physics();
		~Physics();

		void Update();
		void Draw();
		void EnableSimulation(bool enable);

		bool IsEnabled() const;

		btDiscreteDynamicsWorld* GetDynamicsWorld() const;

	private:
		void AddGroundShape();

	private:
		btBroadphaseInterface* mBroadphase;
		btCollisionDispatcher* mDispatcher;
		btConstraintSolver* mConstraintSolver;
		btDefaultCollisionConfiguration* mCollisionConfiguration;
		btDiscreteDynamicsWorld* mDynamicsWorld;

		const glm::vec3 mGravity = glm::vec3(0, -982., 0);
		bool mEnabled;
		// Note: This should be handled by the Timer component.
		// But since the calls to ImGui::NewFrame() and ImGui::Render() currently are not 
		// called at the same frequency as UIOverlay::Update.
		std::chrono::high_resolution_clock::time_point mLastFrameTime;
	};

	Physics& gPhysics();
}
