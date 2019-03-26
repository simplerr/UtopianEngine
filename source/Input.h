#pragma once
#include <Windows.h>
#include <glm/glm.hpp>
#include <functional>
#include "utility/Module.h"

namespace Utopian
{
	//! Wrapper for input with keyboard and mouse.
	class Input : public Module<Input>
	{
	public:
		Input();
		~Input();

		void Update(float dt);
		void Draw();
		void Poll();

		bool KeyPressed(int key, bool excludeUi = true);
		bool KeyDown(int key, bool excludeUi = true);
		bool KeyReleased(int key, bool excludeUi = true);

		//Ray GetWorldPickingRay();

		glm::vec2 GetMousePosition();
		float	MouseDx();
		float	MouseDy();
		float	MouseDz();
		void	SetMousePosition(glm::vec3 pos);

		LRESULT HandleMessages(UINT msg, WPARAM wParam, LPARAM lParam);

		/** Registers a callback function which returns true if the mouse cursor is inside the UI. */
		template<class ...Args>
		void RegisterUiCallback(Args &&...args)
		{
			mIsMouseInsideUiCallback = std::bind(std::forward<Args>(args)...);
		}
	private:
		unsigned char mLastKeyState[256];
		unsigned char mKeyState[256];

		glm::vec2 mMousePosition;
		glm::vec2 mMouseDelta;
		float mMouseWheelDelta;

		std::function<bool()> mIsMouseInsideUiCallback;
	};

	Input& gInput();
}