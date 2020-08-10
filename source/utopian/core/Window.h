#pragma once

#include "utility/Platform.h"
#include <string>

namespace Utopian
{
	class Window
	{
	public:
		Window(int width, int height);
		void SetTitle(std::string title);

		int GetWidth() const;
		int GetHeight() const;

		HWND SetupWindow(HINSTANCE hInstance, WNDPROC wndProc);
		HWND GetHwnd();
		HINSTANCE GetInstance();

	private:
		HWND mWindow;
		HINSTANCE mWindowInstance;
		int mWidth;
		int mHeight;
	};
}
