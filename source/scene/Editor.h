#pragma once
#include "vulkan/VulkanInclude.h"

namespace Scene
{
	class World;
	class Actor;

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
		Actor* mSelectedEntity;
	};
}
