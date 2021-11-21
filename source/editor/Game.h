#pragma once

#include <string>
#include <glm/glm.hpp>
#include <vector>
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"

namespace Utopian
{
   class Editor;
}

class Game
{
public:
   Game(Utopian::Window* window);
   ~Game();

   void Run();

   void DestroyCallback();
   void UpdateCallback(double deltaTime);
   void DrawCallback();
   void PreFrameCallback();

   virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
   void InitScene();
   void AddGround();
   void AddBox(glm::vec3 position, std::string texture);

   // Move all of these to other locations
   SharedPtr<Utopian::Editor> mEditor;
   Utopian::Window* mWindow;
};
