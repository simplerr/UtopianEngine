#include "Input.h"
#include "Camera.h"

//! Constructor.
Input::Input()
{
	// Set every key/button 0.
	ZeroMemory(mLastKeyState, sizeof(mLastKeyState));
	ZeroMemory(mKeyState, sizeof(mKeyState));

	// Get the cursor starting position.
	POINT mousePosition;
	GetCursorPos(&mousePosition);
	mMousePosition.x = mousePosition.x;
	mMousePosition.y = mousePosition.y;

	// No delta movement to start with
	mDx = mDy = 0.0f;
}

//! Cleanup.
Input::~Input()
{

}

//! Update the key state.
/**
@param dt The delta time since the last frame.
*/
void Input::Update(float dt)
{
	// Set the old states.
	memcpy(mLastKeyState, mKeyState, sizeof(mKeyState));

	// Get the current keyboard state.
	GetKeyboardState(mKeyState);
}

//! Draws debug information.
void Input::Draw()
{

}

//! Updates the mouse position.
/**
@param msg The message.
@param wParam Message data.
@param lParam Message data.
*/
LRESULT Input::HandleMessages(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_MOUSEMOVE: {
		SetMousePosition(vec3(LOWORD(lParam), HIWORD(lParam), 0));
		return 0;
	}
	case WM_LBUTTONDOWN: {
		return 0;
	}
	}
}

//! Checks if the key was pressed.
/**
@param key The to check.
@return True if pressed.
*/
bool Input::KeyPressed(int key)
{
	if (key > 255 || key < 0)
		return false;

	// Usees bitwise AND to remove the least significant bit which is set if the key was pressed after the previous call to GetAsyncKeyState.
	// See http://msdn.microsoft.com/en-us/library/windows/desktop/ms646293(v=VS.85).aspx for more info.
	// The same thing in keyDown() and keyReleased()
	return (mKeyState[key] & 0x80) && !(mLastKeyState[key] & 0x80);
}

//! Checks if the key is down.
/**
@param key The to check.
@return True if down.
*/
bool Input::KeyDown(int key)
{
	if (key > 255 || key < 0)
		return false;

	return mKeyState[key] & 0x80;
}

//! Checks if the key was released.
/**
@param key The to check.
@return True if released.
*/
bool Input::KeyReleased(int key)
{
	if (key > 255 || key < 0)
		return false;

	return !(mKeyState[key] & 0x80) && (mLastKeyState[key] & 0x80);
}

//! Get the mouse position
/**
@return The mouse position.
*/
vec3 Input::GetMousePosition()
{
	return mMousePosition;
}

//! Set the mouse position.
/**
@param pos The new position.
@note Doesn't acctually change the mouse position on the screen, just the data.
*/
void Input::SetMousePosition(vec3 pos)
{
	mDx = pos.x - GetMousePosition().x;
	mDy = pos.y - GetMousePosition().y;

	mMousePosition = pos;
}

//! Returns horizontal delta movement.
float Input::MouseDx()
{
	return -1;
}

//! Returns vertical delta movement.
float Input::MouseDy()
{
	return -1;
}

float Input::MouseDz()
{
	return -1;
}

void Input::Poll()
{
}
