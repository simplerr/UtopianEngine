#include "utility/Platform.h"
#include "core/Window.h"
#include "Game.h"

Game* gGame = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (gGame != nullptr)
		gGame->HandleMessages(hwnd, msg, wParam, lParam);

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	Utopian::Window window = Utopian::Window("data/window.cfg");

	window.SetupWindow(hInstance, WndProc);

	gGame = new Game(&window);
	gGame->Run();

	delete gGame;

	return 0;
}

