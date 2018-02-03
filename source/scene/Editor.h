#pragma once
#include "vulkan/VulkanInclude.h"

namespace Scene
{
	class World;
	class SceneEntity;

	class Editor
	{
	public:
		Editor(Vulkan::Renderer* renderer, World* world);
		~Editor();

		void Update();
		void UpdateUi();

		void Draw();
	private:
		bool IsEntitySelected();

		Vulkan::Renderer* mRenderer;
		World* mWorld;
		SceneEntity* mSelectedEntity;
	};
}
