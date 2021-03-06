#include "core/Window.h"
#include <sstream>
#include <fstream>
#include <string.h>

namespace Utopian
{
   Window::Window(int width, int height)
   {
      mWidth = width;
      mHeight = height;
      mMode = WINDOWED;
   }

   Window::Window(std::string configFile)
   {
      std::ifstream fin(configFile);
      uint32_t width, height;
      std::string mode;
      fin >> width >> height >> mode;

      if (mode == "windowed") {
         mMode = WINDOWED;
      }
      else if (mode == "fullscreen-windowed") {
         mMode = FULLSCREEN_WINDOWED;
         width = GetSystemMetrics(SM_CXSCREEN);
         height = GetSystemMetrics(SM_CYSCREEN);
      }

      mWidth = width;
      mHeight = height;
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
      clientRect.left = GetSystemMetrics(SM_CXSCREEN) / 2 - mWidth / 2;
      clientRect.right = GetSystemMetrics(SM_CXSCREEN) / 2 + mWidth / 2;
      clientRect.top = GetSystemMetrics(SM_CYSCREEN) / 2 - mHeight / 2;
      clientRect.bottom = GetSystemMetrics(SM_CYSCREEN) / 2 + mHeight / 2;

      DWORD style;
      if (mMode == WINDOWED)
         style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
      else if(mMode == FULLSCREEN_WINDOWED)
         style = WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_CLIPCHILDREN;

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

   bool Window::DispatchMessages()
   {
      bool closeWindow = false;

      MSG msg;
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
         if (msg.message == WM_QUIT)
         {
            closeWindow = true;
         }
         else
         {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
         }
      }

      return closeWindow;
   }

   void Window::SetTitle(std::string title)
   {
      std::stringstream ss;
      ss << title;
      std::string windowTitle = ss.str();
      SetWindowText(GetHwnd(), windowTitle.c_str());
   }


   HWND Window::GetHwnd()
   {
      return mWindow;
   }

   HINSTANCE Window::GetInstance()
   {
      return mWindowInstance;
   }

   int Window::GetWidth() const
   {
      return mWidth;
   }

   int Window::GetHeight() const
   {
      return mHeight;
   }

}