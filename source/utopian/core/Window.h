#pragma once

#include "utility/Platform.h"
#include <string>

namespace Utopian
{
	class Window
	{
	public:
		enum WindowMode
		{
			WINDOWED,
			FULLSCREEN_WINDOWED
		};

		Window(int width, int height);
		Window(std::string configFile);

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
		WindowMode mMode;
		int mWidth;
		int mHeight;
	};
}
