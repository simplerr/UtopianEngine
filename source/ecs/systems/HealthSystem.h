#pragma once
#include <window.h>
#include <vector>
#include "System.h"

namespace ECS
{
	class HealthComponent;
	class EntityManager;

	/*
		\brief The responsibility of the HealthSystem is to check which entities
		that have <= 0 health and then inform EntityManager so they can be removed
	*/
	class HealthSystem : public System
	{
	public:
		HealthSystem(EntityManager* entityManager);

		void Process();
		void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
	};
}
