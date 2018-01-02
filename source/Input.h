#pragma once
#include <Windows.h>
#include <glm/glm.hpp>

//! Wrapper for input with keyboard and mouse.
class Input
{
public:
	Input();
	~Input();

	void Update(float dt);
	void Draw();
	void Poll();

	bool KeyPressed(int key);
	bool KeyDown(int key);
	bool KeyReleased(int key);

	//Ray GetWorldPickingRay();

	glm::vec3 GetMousePosition();
	float	MouseDx();
	float	MouseDy();
	float	MouseDz();
	void	SetMousePosition(glm::vec3 pos);

	LRESULT HandleMessages(UINT msg, WPARAM wParam, LPARAM lParam);
private:
	unsigned char mLastKeyState[256];
	unsigned char mKeyState[256];

	glm::vec3 mMousePosition;
	float mDx, mDy;
};