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

	enum ActorTemplate
	{
		STATIC_MODEL,
		LIGHT
	};

	class Editor
	{
	public:
		Editor(Utopian::Vk::Renderer* renderer, World* world, Terrain* terrain);
		~Editor();

		void Update();
		void UpdateUi();
		void Draw();

		void AddActorCreation(std::string path, ActorTemplate actorTemplate = STATIC_MODEL);
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

		/*
		 * The editor supports creating Actors from different templates consisting
		 * of different default components. For example STATIC_MODEL is an actor with
		 * a CTransfrom + CRenderable. Todo: Note: This should be improved.
		*/
		std::vector<ActorTemplate> mTemplateTypes;
		int mSelectedModel = 0;
	};
}
