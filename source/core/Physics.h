#pragma once
#include "utility/Module.h"
#include <glm/glm.hpp>

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

		btDiscreteDynamicsWorld* GetDynamicsWorld() const;

	private:
		btBroadphaseInterface* mBroadphase;
		btCollisionDispatcher* mDispatcher;
		btConstraintSolver* mConstraintSolver;
		btDefaultCollisionConfiguration* mCollisionConfiguration;
		btDiscreteDynamicsWorld* mDynamicsWorld;

		const glm::vec3 mGravity = glm::vec3(0, -982., 0);
	};

	Physics& gPhysics();
}
