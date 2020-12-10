#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "core/World.h"
#include "editor/Editor.h"

namespace Utopian
{
   class World;
   class Actor;
   class CPolyMesh;

   class PrototypeTool
   {
   public:
      PrototypeTool();
      ~PrototypeTool();

      void Update(World* world, Actor* selectedActor);
      void PreFrame();
      void RenderUi();

      void ActorSelected(Actor* actor);
      void SetSelectionType(SelectionType selectionType);
    private:
      void DrawFaceGizmo();
      void DrawEdgeGizmo();
      void UpdateRigidBody();
    private:
      Actor* mSelectedActor = nullptr;
      CPolyMesh* mSelectedMesh = nullptr;
      SelectionType mSelectionType = FACE_SELECTION;
      bool mSelected = false;
   };
}