#pragma once
#include "vulkan/VulkanInclude.h"

class TransformTool;
class Terrain;

namespace Scene
{
	class World;
	class Actor;
	class ActorInspector;

	class Editor
	{
	public:
		Editor(Vulkan::Renderer* renderer, World* world, Terrain* terrain);
		~Editor();

		void Update();
		void UpdateUi();

		void Draw();
	private:
		bool IsActorSelected();
		void OnActorSelected(Actor* actor);

		Vulkan::Renderer* mRenderer;
		World* mWorld;
		Terrain* mTerrain;
		ActorInspector* mActorInspector;
		TransformTool* mTransformTool;
		Actor* mSelectedActor;
	};
}
