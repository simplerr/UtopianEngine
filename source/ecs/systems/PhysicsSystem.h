#pragma once

#include <vector>
#include <window.h>
#include "System.h"

namespace Vulkan
{
}

namespace ECS
{
	class TransformComponent;
	class PhysicsComponent;


	class PhysicsSystem : public System
	{
	public:
		PhysicsSystem(EntityManager* entityManager);
		void Process();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
	};
}