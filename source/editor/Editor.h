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
	class TerrainTool;
	class FoliageTool;
	class ImGuiRenderer;

	enum ActorTemplate
	{
		STATIC_MODEL,
		STATIC_POINT_LIGHT,
		RIGID_BOX,
		RIGID_SPHERE,
		RIGID_SPHERE_LIGHT
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
		void OnActorSelected(Actor* actor);
		void RenderActorCreationUi();
		void RenderActorSelectionUi();

		void AddPaths();

		World* mWorld;
		SharedPtr<Terrain> mTerrain;
		Camera* mCamera;
		ImGuiRenderer* mImGuiRenderer;
		ActorInspector* mActorInspector;
		SharedPtr<TerrainTool> mTerrainTool;
		SharedPtr<FoliageTool> mFoliageTool;
		Actor* mSelectedActor;
		int mSelectedActorIndex = 0;

		std::vector<const char*> mModelPaths;

		/*
		 * The editor supports creating Actors from different templates consisting
		 * of different default components. For example STATIC_MODEL is an actor with
		 * a CTransfrom + CRenderable. Todo: Note: This should be improved.
		*/
		std::vector<ActorTemplate> mTemplateTypes;
		int mSelectedModel = 0;

		const float MWHEEL_SCALE_FACTOR = 0.025f;
	};
}
