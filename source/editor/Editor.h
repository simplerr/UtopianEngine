#pragma once
#include "vulkan/VulkanInclude.h"
#include <vector>
#include <string>

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

		void AddModelPath(std::string path);
	private:
		bool IsActorSelected();
		void OnActorSelected(Actor* actor);

		void AddPaths();

		Utopian::Vk::Renderer* mRenderer;
		World* mWorld;
		Terrain* mTerrain;
		ActorInspector* mActorInspector;
		TransformTool* mTransformTool;
		Actor* mSelectedActor;

		std::vector<const char*> mModelPaths;
		int mSelectedModel = 0;
	};
}
