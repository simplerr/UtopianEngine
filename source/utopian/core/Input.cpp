#include "core/Input.h"
#include "core/Camera.h"
#include "core/Engine.h"
#include "vulkan/VulkanApp.h"
#include "imgui/imgui.h"

namespace Utopian
{
   Input& gInput()
   {
      return Input::Instance();
   }

   //! Constructor.
   Input::Input()
   {
      // Set every key/button 0.
      ZeroMemory(mLastKeyState, sizeof(mLastKeyState));
      ZeroMemory(mKeyState, sizeof(mKeyState));

      // Get the cursor starting position.
      POINT mousePosition;
      GetCursorPos(&mousePosition);
      mMousePosition.x = (float)mousePosition.x;
      mMousePosition.y = (float)mousePosition.y;

      // No delta movement to start with
      mMouseDelta = glm::vec2(0.0f);
      mMousePosition = glm::vec2(-1.0f);
      mMouseWheelDelta = 0.0f;

      mVisibleCursor = true;
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
      mMouseDelta = glm::vec2(0.0f);
      mMouseWheelDelta = 0.0f;

      // Set the old states.
      memcpy(mLastKeyState, mKeyState, sizeof(mKeyState));

      // Get the current keyboard state.
      (void)GetKeyboardState(mKeyState);
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
      case WM_MOUSEMOVE:
      {
         int x = LOWORD(lParam);
         int y = HIWORD(lParam);

         if (mMousePosition.x == -1 && mMousePosition.y == -1) {
            mMousePosition.x = (float)x;
            mMousePosition.y = (float)y;
            break;
         }

         mMouseDelta.x = x - mMousePosition.x;
         mMouseDelta.y = mMousePosition.y - y;     // Other way around

         mMousePosition.x = (float)x;
         mMousePosition.y = (float)y;

         if (!mVisibleCursor)
            LockCursorPosition();

         break;
      }
      case WM_MOUSEWHEEL:
         mMouseWheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);
         break;
      case WM_KEYDOWN:
      {
         char key = (char)wParam;
         if (IsLetter((char)wParam))
         {
            // Change to non captial letter
            if (!KeyDown(VK_SHIFT, false))
               key += 32;
         }

         mKeydownCallback(key);
         break;
      }
      }

      return 0;
   }

   void Input::LockCursorPosition()
   {
      Utopian::Window* window = Utopian::gEngine().GetVulkanApp()->GetWindow();
      HWND hwnd = window->GetHwnd();

      if (GetFocus() == hwnd)
      {
         int width = window->GetWidth();
         int height = window->GetHeight();
         POINT forcedPos = {width / 2, height / 2};
         ClientToScreen(hwnd, &forcedPos);

         SetCursorPos(forcedPos.x, forcedPos.y);
         mMousePosition.x = width / 2.0f;
         mMousePosition.y = height / 2.0f;
      }
   }

   //! Checks if the key was pressed.
   /**
   @param key The to check.
   @return True if pressed.
   */
   bool Input::KeyPressed(int key, bool excludeUi)
   {
      if (key > 255 || key < 0)
         return false;

      // Usees bitwise AND to remove the least significant bit which is set if the key was pressed after the previous call to GetAsyncKeyState.
      // See http://msdn.microsoft.com/en-us/library/windows/desktop/ms646293(v=VS.85).aspx for more info.
      // The same thing in keyDown() and keyReleased()
      bool keyPressed = (mKeyState[key] & 0x80) && !(mLastKeyState[key] & 0x80);

      if (key == VK_LBUTTON || key == VK_RBUTTON)
      {
         if (excludeUi && mIsMouseInsideUiCallback())
         {
            keyPressed = false;
         }
      }
      // If keyboard input is captured by the UI then don't return true.
      else if (excludeUi && mIsUiCapturingKeyboardCallback())
      {
         keyPressed = false;
      }

      return keyPressed;
   }

   //! Checks if the key is down.
   /**
   @param key The to check.
   @return True if down.
   */
   bool Input::KeyDown(int key, bool excludeUi)
   {
      if (key > 255 || key < 0)
         return false;

      bool keyDown = mKeyState[key] & 0x80;
   
      if (key == VK_LBUTTON || key == VK_RBUTTON)
      {
         if (excludeUi && mIsMouseInsideUiCallback())
         {
            keyDown = false;
         }
      }
      // If keyboard input is captured by the UI then don't return true.
      else if (excludeUi && mIsUiCapturingKeyboardCallback())
      {
         keyDown = false;
      }

      return keyDown;
   }

   //! Checks if the key was released.
   /**
   @param key The to check.
   @return True if released.
   */
   bool Input::KeyReleased(int key, bool excludeUi)
   {
      if (key > 255 || key < 0)
         return false;

      bool keyReleased = !(mKeyState[key] & 0x80) && (mLastKeyState[key] & 0x80);

      if (key == VK_LBUTTON || key == VK_RBUTTON)
      {
         if (excludeUi && mIsMouseInsideUiCallback())
         {
            keyReleased = false;
         }
      }
      // If keyboard input is captured by the UI then don't return true.
      else if (excludeUi && mIsUiCapturingKeyboardCallback())
      {
         keyReleased = false;
      }

      return keyReleased;
   }

   //! Get the mouse position
   /**
   @return The mouse position.
   */
   glm::vec2 Input::GetMousePosition()
   {
      return mMousePosition;
   }

   //! Set the mouse position.
   /**
   @param pos The new position.
   @note Doesn't acctually change the mouse position on the screen, just the data.
   */
   void Input::SetMousePosition(glm::vec3 pos)
   {
      //mDx = pos.x - GetMousePosition().x;
      //mDy = pos.y - GetMousePosition().y;

      mMousePosition = pos;
   }

   //! Returns horizontal delta movement.
   float Input::MouseDx()
   {
      return mMouseDelta.x;
   }

   //! Returns vertical delta movement.
   float Input::MouseDy()
   {
      return mMouseDelta.y;
   }

   float Input::MouseDz()
   {
      return mMouseWheelDelta;
   }

   void Input::Poll()
   {
   }

   void Input::RegisterMouseInsideUiCallback(std::function<bool(void)> callback)
   {
      mIsMouseInsideUiCallback = callback;
   }

   void Input::RegisterKeydownCallback(std::function<void(char)> callback)
   {
      mKeydownCallback = callback;
   }

   void Input::RegisterUiCaptureCallback(std::function<bool(void)> callback)
   {
      mIsUiCapturingKeyboardCallback = callback;
   }

   bool Input::IsLetter(char key)
   {
      return (key >= 65 && key <= 90);
   }

   void Input::SetVisibleCursor(bool visible)
   {
      mVisibleCursor = visible;
      ShowCursor(mVisibleCursor);
   }
}
