#pragma once
#include "vulkan/VulkanInclude.h"

namespace Scene
{
	class World;
	class Actor;
	class ActorInspector;

	class Editor
	{
	public:
		Editor(Vulkan::Renderer* renderer, World* world);
		~Editor();

		void Update();
		void UpdateUi();

		void Draw();
	private:
		bool IsActorSelected();
		void OnActorSelected(Actor* actor);

		Vulkan::Renderer* mRenderer;
		World* mWorld;
		ActorInspector* mActorInspector;
		Actor* mSelectedActor;
	};
}
