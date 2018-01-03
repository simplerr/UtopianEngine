#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vector>
#include <glm/glm.hpp>
#include <window.h>
#include "System.h"
#include "Collision.h"

namespace Vulkan
{
	class Renderer;
	class Camera;
}

class Terrain;
class ObjectTool;
class Input;

namespace ECS
{
	class TransformComponent;
	class MeshComponent;
	class HealthComponent;

	/*
	Primary functionality:
	- Adding entities to the terrain
	- Moving entities (x, y, z axis)
	- Moving entities attached to mouse ray
	- Scaling
	- Saving and loading of entities to file
	- Snapping to terrain height

	Secondary functionality:
	- Adding lights
	- Adding/removing Components to entities
	- Changing properties through UI

	*/
	class EditorSystem : public System
	{
	public:
		EditorSystem(SystemManager* entityManager, Vulkan::Camera* camera, Terrain* terrain, Input* input);
		void Process();
		void PerformPicking();

		Entity* GetPickedEntity();

		void SaveToFile();
		void LoadFromFile();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		Vulkan::Camera* mCamera;
		Terrain* mTerrain;
		Input* mInput;
		ObjectTool* mObjectTool;
		int32_t mPickedId;

		const float SCALING_SPEED = 2.0f;
	};
}
