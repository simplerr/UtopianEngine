#pragma once
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"
#include <vector>
#include <string>

namespace Utopian
{
	class World;
	class Actor;
	class ActorInspector;
	class Terrain;
	class TransformTool;
	class TerrainTool;
	class FoliageTool;
	class ImGuiRenderer;

	enum ActorTemplate
	{
		STATIC_MODEL,
		STATIC_POINT_LIGHT,
		MOVING_POINT_LIGHT
	};

	class Editor
	{
	public:
		Editor(ImGuiRenderer* imGuiRenderer, Camera* camera, World* world, const SharedPtr<Terrain>& terrain);
		~Editor();

		void Update();
		void UpdateUi();
		void Draw();

		void AddActorCreation(std::string path, ActorTemplate actorTemplate = STATIC_MODEL);
	private:
		bool IsActorSelected();
		void UnselectActor();
		void OnActorSelected(Actor* actor);
		void RenderActorCreationUi();

		void AddPaths();

		World* mWorld;
		SharedPtr<Terrain> mTerrain;
		Camera* mCamera;
		ImGuiRenderer* mImGuiRenderer;
		ActorInspector* mActorInspector;
		SharedPtr<TransformTool> mTransformTool;
		SharedPtr<TerrainTool> mTerrainTool;
		SharedPtr<FoliageTool> mFoliageTool;
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
