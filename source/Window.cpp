#include "Window.h"

namespace Vulkan
{
	Window::Window(int width, int height)
	{
		mWidth = width;
		mHeight = height;
	}

	int Window::GetWidth()
	{
		return mWidth;
	}

	int Window::GetHeight()
	{
		return mHeight;
	}

#if defined(_WIN32)
	// Creates a window that Vulkan can use for rendering
	HWND Window::SetupWindow(HINSTANCE hInstance, WNDPROC WndProc)
	{
		mWindowInstance = hInstance;

		WNDCLASS wc;
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = mWindowInstance;
		wc.hIcon = LoadIcon(0, IDI_APPLICATION);
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName = 0;
		wc.lpszClassName = "VulkanWndClassName";

		if (!RegisterClass(&wc)) {
			MessageBox(0, "RegisterClass FAILED", 0, 0);
			PostQuitMessage(0);
		}

		RECT clientRect;
		clientRect.left = GetSystemMetrics(SM_CXSCREEN) / 2 - mWidth / 2.0f;
		clientRect.right = GetSystemMetrics(SM_CXSCREEN) / 2 + mWidth / 2.0f;
		clientRect.top = GetSystemMetrics(SM_CYSCREEN) / 2 - mHeight / 2.0f;
		clientRect.bottom = GetSystemMetrics(SM_CYSCREEN) / 2 + mHeight / 2.0f;

		DWORD style = true ? WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN : WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_CLIPCHILDREN;

		AdjustWindowRect(&clientRect, style, false);
		int width = clientRect.right - clientRect.left;
		int height = clientRect.bottom - clientRect.top;

		// Create the window with a custom size and make it centered
		// NOTE: WS_CLIPCHILDREN Makes the area under child windows not be displayed. (Useful when rendering DirectX and using windows controls).
		mWindow = CreateWindow("VulkanWndClassName", "Vulkan App",
			style, GetSystemMetrics(SM_CXSCREEN) / 2 - (mWidth / 2),
			GetSystemMetrics(SM_CYSCREEN) / 2 - (mHeight / 2), width, height,
			0, 0, mWindowInstance, 0);

		if (!mWindow) {
			auto error = GetLastError();
			MessageBox(0, "CreateWindow() failed.", 0, 0);
			PostQuitMessage(0);
		}

		// Show the newly created window.
		ShowWindow(mWindow, SW_SHOW);
		SetForegroundWindow(mWindow);
		SetFocus(mWindow);

		return mWindow;
	}

	HWND Window::GetHwnd()
	{
		return mWindow;
	}

	HINSTANCE Window::GetInstance()
	{
		return mWindowInstance;
	}

#elif defined(__linux__)
	// Set up a window using XCB and request event types
	xcb_window_t Window::SetupWindow()
	{
		uint32_t value_mask, value_list[32];

		mWindow = xcb_generate_id(mConnection);

		value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
		value_list[0] = mScreen->black_pixel;
		value_list[1] =
			XCB_EVENT_MASK_KEY_RELEASE |
			XCB_EVENT_MASK_EXPOSURE |
			XCB_EVENT_MASK_STRUCTURE_NOTIFY |
			XCB_EVENT_MASK_POINTER_MOTION |
			XCB_EVENT_MASK_BUTTON_PRESS |
			XCB_EVENT_MASK_BUTTON_RELEASE;

		xcb_create_window(mConnection,
			XCB_COPY_FROM_PARENT,
			mWindow, mScreen->root,
			0, 0, width, height, 0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT,
			mScreen->root_visual,
			value_mask, value_list);

		/* Magic code that will send notification when window is destroyed */
		xcb_intern_atom_cookie_t cookie = xcb_intern_atom(mConnection, 1, 12, "WM_PROTOCOLS");
		xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(mConnection, cookie, 0);

		xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(mConnection, 0, 16, "WM_DELETE_WINDOW");
		atom_wm_delete_window = xcb_intern_atom_reply(mConnection, cookie2, 0);

		xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE,
			mWindow, (*reply).atom, 4, 32, 1,
			&(*atom_wm_delete_window).atom);

		std::string windowTitle = getWindowTitle();
		xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE,
			mWindow, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
			title.size(), windowTitle.c_str());

		free(reply);

		xcb_map_window(mConnection, mWindow);

		return(mWindow);
	}

	// Initialize XCB connection
	void Window::InitxcbConnection()
	{
		const xcb_setup_t *setup;
		xcb_screen_iterator_t iter;
		int scr;

		mConnection = xcb_connect(NULL, &scr);
		if (mConnection == NULL) {
			printf("Could not find a compatible Vulkan ICD!\n");
			fflush(stdout);
			exit(1);
		}

		setup = xcb_get_setup(mConnection);
		iter = xcb_setup_roots_iterator(setup);
		while (scr-- > 0)
			xcb_screen_next(&iter);
		mScreen = iter.data;
	}

	void Window::HandleEvent(const xcb_generic_event_t *event)
	{
		// TODO
	}

	xcb_window_t Window::GetWindow()
	{
		return mWindow;
	}

	xcb_connection_t * Window::GetConnection()
	{
		return mConnection;
	}

#endif

}	// VulkanLib namespace