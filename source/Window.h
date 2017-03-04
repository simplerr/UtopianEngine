#pragma once

#include "Platform.h"

namespace Vulkan
{
	// Wrapper class for the platform dependent window code
	class Window
	{
	public:

		Window(int width, int height);

		int GetWidth();
		int GetHeight();

		// Platform specific
#if defined(_WIN32)
		HWND SetupWindow(HINSTANCE hInstance, WNDPROC wndProc);
		HWND GetHwnd();
		HINSTANCE GetInstance();
#elif defined(__linux__)
		xcb_window_t SetupWindow();
		xcb_window_t GetWindow();
		xcb_connection_t* GetConnection();
		void InitxcbConnection();
		void HandleEvent(const xcb_generic_event_t *event);

#endif

	private:
		// Platform specific 
#if defined(_WIN32)
		HWND				mWindow;
		HINSTANCE			mWindowInstance;
#elif defined(__linux__)
		bool quit;
		xcb_connection_t *mConnection;
		xcb_screen_t *mScreen;
		xcb_window_t mWindow;
		xcb_intern_atom_reply_t *atom_wm_delete_window;
#endif

		int mWidth;
		int mHeight;
	};
}	// VulkanLib namespace
