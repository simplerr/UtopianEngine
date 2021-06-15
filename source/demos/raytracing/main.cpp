
#include "utility/Platform.h"
#include "core/Window.h"
#include "RayTrace.h"

RayTrace* gApp = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   if (gApp != nullptr)
      gApp->HandleMessages(hwnd, msg, wParam, lParam);
   else
      return DefWindowProc(hwnd, msg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
   Utopian::Window window = Utopian::Window("data/window.cfg");

   window.SetupWindow(hInstance, WndProc);

   gApp = new RayTrace(&window);
   gApp->Run();

   delete gApp;

   return 0;
}

