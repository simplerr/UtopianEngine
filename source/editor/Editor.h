#pragma once
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "ui/Console.h"

namespace Utopian
{
   class World;
   class Actor;
   class ActorInspector;
   class Terrain;
   class TerrainTool;
   class FoliageTool;
   class PrototypeTool;
   class ImGuiRenderer;

   enum ActorTemplate
   {
      STATIC_MODEL,
      STATIC_POINT_LIGHT,
      RIGID_BOX,
      RIGID_SPHERE,
      RIGID_SPHERE_LIGHT,
      SPAWN_POINT,
      FINISH_POINT,
      PBR_SPHERE
   };

   enum SelectionType
   {
      OBJECT_SELECTION,
      FACE_SELECTION,
      EDGE_SELECTION
   };

   class Editor
   {
   public:
      Editor(ImGuiRenderer* imGuiRenderer, World* world, Terrain* terrain);
      ~Editor();

      void Update(double deltaTime);
      void UpdateUi();
      void Draw();
      void PreFrame();

      void CreateActor(std::string modelPath, ActorTemplate actorTemplate, glm::vec3 position);
      void AddActorCreation(std::string modelPath, ActorTemplate actorTemplate = STATIC_MODEL);
   private:
      void UpdateSelectionType();
      void DrawGizmo();
      void SelectActor();
      void AddActorToScene();
      void RemoveActorFromScene();
      void ScaleSelectedActor();

      bool IsActorSelected();
      void OnActorSelected(Actor* actor);
      void DeselectActor();
      void RenderActorCreationUi();
      void RenderActorSelectionUi();
      void RenderLoadSaveUi();
      void SaveTerrain();
      void LoadTerrain();

      void AddPaths();

      World* mWorld;
      Terrain* mTerrain;
      ImGuiRenderer* mImGuiRenderer;
      ActorInspector* mActorInspector;
      SharedPtr<TerrainTool> mTerrainTool;
      SharedPtr<FoliageTool> mFoliageTool;
      SharedPtr<PrototypeTool> mPrototypeTool;
      Actor* mSelectedActor;
      Console mConsole;
      SelectionType mSelectionType = OBJECT_SELECTION;
      int mSelectedActorIndex = 0;

      std::vector<const char*> mModelPaths;

      /*
       * The editor supports creating Actors from different templates consisting
       * of different default components. For example STATIC_MODEL is an actor with
       * a CTransfrom + CRenderable. Todo: Note: This should be improved.
      */
      std::vector<ActorTemplate> mTemplateTypes;
      int mSelectedModel = 0;

      const float MWHEEL_SCALE_FACTOR = 0.0025f;
   };
}
