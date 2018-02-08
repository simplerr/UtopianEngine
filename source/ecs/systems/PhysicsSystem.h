#pragma once

#include <vector>
#include <window.h>
#include "System.h"

namespace Utopian::Vk
{
}

namespace ECS
{
	class TransformComponent;
	class PhysicsComponent;


	class PhysicsSystem : public System
	{
	public:
		PhysicsSystem();
		void Process();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
	};
}