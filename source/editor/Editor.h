#pragma once
#include "vulkan/VulkanInclude.h"

class TransformTool;
class Terrain;

namespace Utopian
{
	class World;
	class Actor;
	class ActorInspector;

	class Editor
	{
	public:
		Editor(Utopian::Vk::Renderer* renderer, World* world, Terrain* terrain);
		~Editor();

		void Update();
		void UpdateUi();

		void Draw();
	private:
		bool IsActorSelected();
		void OnActorSelected(Actor* actor);

		Utopian::Vk::Renderer* mRenderer;
		World* mWorld;
		Terrain* mTerrain;
		ActorInspector* mActorInspector;
		TransformTool* mTransformTool;
		Actor* mSelectedActor;
	};
}
