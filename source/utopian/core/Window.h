#pragma once

#include "utility/Platform.h"
#include <string>

namespace Utopian
{
	class Window
	{
	public:
		Window(int width, int height);

		HWND SetupWindow(HINSTANCE hInstance, WNDPROC wndProc);

		/** Peeks and dispatches Win32 messages. */
		bool DispatchMessages();

		void SetTitle(std::string title);

		HWND GetHwnd();
		HINSTANCE GetInstance();
		int GetWidth() const;
		int GetHeight() const;

	private:
		HWND mWindow;
		HINSTANCE mWindowInstance;
		int mWidth;
		int mHeight;
	};
}
