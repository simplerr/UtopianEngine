#include "core/Window.h"
#include <sstream>

namespace Utopian
{
	Window::Window(int width, int height)
	{
		mWidth = width;
		mHeight = height;
	}
	
	void Window::SetTitle(std::string title)
	{
		std::stringstream ss;
		ss << title;
		std::string windowTitle = ss.str();
		SetWindowText(GetHwnd(), windowTitle.c_str());
	}

	int Window::GetWidth() const
	{
		return mWidth;
	}

	int Window::GetHeight() const
	{
		return mHeight;
	}

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
}