
#include "utility/Platform.h"
#include "core/Window.h"
#include "MarchingCubes.h"

MarchingCubes* gApp = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   if (gApp != nullptr)
      gApp->HandleMessages(hwnd, msg, wParam, lParam);
   else
      return DefWindowProc(hwnd, msg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
   Utopian::Window window = Utopian::Window(2000, 1260);
   window.SetupWindow(hInstance, WndProc);

   gApp = new MarchingCubes(&window);
   gApp->Run();

   delete gApp;

   return 0;
}

