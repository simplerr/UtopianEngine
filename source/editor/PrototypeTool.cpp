#include "PrototypeTool.h"
#include "core/components/Actor.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/CRigidBody.h"
#include "core/components/CPolyMesh.h"
#include "core/Input.h"
#include "core/Log.h"
#include "core/renderer/Renderer.h"
#include "core/renderer/ImGuiRenderer.h"
#include "core/renderer/Im3dRenderer.h"
#include "core/Camera.h"
#include "core/World.h"
#include "core/physics/Physics.h"
#include "utility/math/Ray.h"
#include "utility/math/Helpers.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/StaticModel.h"
#include "core/Log.h"
#include <OpenMesh/Core/Mesh/Handles.hh>
#include <OpenMesh/Core/Mesh/SmartHandles.hh>
#include <core/components/CRandomPaths.h>
#include <im3d/im3d.h>
#include <vulkan/VulkanPrerequisites.h>

#include <iostream>

namespace Utopian
{
   PrototypeTool::PrototypeTool()
   {
      //gPhysics().EnableDebugDraw(true);
   }

   PrototypeTool::~PrototypeTool()
   {

   }

   void PrototypeTool::PreFrame()
   {
      if (mSelectedMesh != nullptr)
      {
         mSelectedMesh->PreFrame();

         if (gInput().KeyPressed('N') || gInput().KeyPressed('M') || 
            gInput().KeyPressed('T') || gInput().KeyPressed('Y') || 
            gInput().KeyPressed('F') || gInput().KeyPressed('G') || 
            gInput().KeyPressed('J') || gInput().KeyPressed('K') || 
            gInput().KeyPressed('R'))
         {
            CRigidBody* rigidBody = mSelectedActor->GetComponent<CRigidBody>();
            rigidBody->AddToWorld();
         }
      }
   }

   void PrototypeTool::ActorSelected(Actor* actor)
   {
      CRenderable* renderable = actor->GetComponent<CRenderable>();
      mSelectedActor = actor;
      mSelectedMesh = actor->GetComponent<CPolyMesh>();
      mSelected = true;
   }

   void PrototypeTool::Update(World* world, Actor* selectedActor)
   {
      // Add face
      if (gInput().KeyPressed('R'))
      {
         mSelectedMesh->ExtrudeSelectedFace(1.0f);
      }

      if (gInput().KeyPressed(VK_LBUTTON) && gInput().KeyDown(VK_LCONTROL))
      {
         Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
         mSelectedMesh->SelectFace(ray);
      }
      
      if (gInput().KeyPressed('U'))
      {
         mSelectedMesh->WriteToFile("polymesh.obj");
      }

      if (mSelected)
      {
         if (mSelectionType == FACE_SELECTION)
            DrawFaceGizmo();
         else if (mSelectionType == EDGE_SELECTION)
            DrawEdgeGizmo();
      }
   }

   void PrototypeTool::DrawFaceGizmo()
   {
      // Face highlight
      glm::vec3 v0, v1, v2, v3;
      mSelectedMesh->GetSelectedFaceVertices(v0, v1, v2, v3);
      Im3d::PushColor(Im3d::Color_Yellow);
      Im3d::DrawQuadFilled(v0, v1, v2, v3);
      Im3d::PopColor();

      glm::vec3 faceCenter = mSelectedMesh->GetSelectedFaceCenter();
      glm::mat4 transform = glm::translate(glm::mat4(), faceCenter);
      Im3d::Mat4 im3dTransform = Im3d::Mat4(transform);
      static glm::vec3 prevScale = glm::vec3(1.0f);

      if (Im3d::Gizmo("FaceGizmo", im3dTransform))
      {
         glm::vec3 newFaceCenter = im3dTransform.getTranslation();
         glm::vec3 delta = newFaceCenter - faceCenter;

         // Translate delta to model space
         glm::mat4 world = mSelectedActor->GetTransform().GetWorldMatrix();
         delta = glm::inverse(world) * glm::vec4(delta, 0.0f);
         mSelectedMesh->MoveSelectedFace(delta);

         glm::vec3 newScale = im3dTransform.getScale();
         glm::vec3 deltaScale = newScale - prevScale;
         mSelectedMesh->ScaleSelectedFace(deltaScale.x / 1.0f);

         prevScale.x = newScale.x;
      }
      else
      {
         prevScale = glm::vec3(1.0f);
      }
   }

   void PrototypeTool::DrawEdgeGizmo()
   {
      // Edge highlight
      glm::vec3 v0, v1;
      mSelectedMesh->GetSelectedEdgeVertices(v0, v1);
      Im3d::DrawLine(v0, v1, 5.0f, Im3d::Color_Green);

      glm::vec3 delta = v1 - v0;
      glm::vec3 edgeCenter = v0 + delta * 0.5f;

      glm::mat4 transform = glm::translate(glm::mat4(), edgeCenter);
      Im3d::Mat4 im3dTransform = Im3d::Mat4(transform);
      if (Im3d::Gizmo("EdgeGizmo", im3dTransform))
      {
         glm::vec3 newFaceCenter = im3dTransform.getTranslation();
         glm::vec3 delta = newFaceCenter - edgeCenter;

         // Translate delta to model space
         glm::mat4 world = mSelectedActor->GetTransform().GetWorldMatrix();
         delta = glm::inverse(world) * glm::vec4(delta, 0.0f);
         mSelectedMesh->MoveSelectedEdge(delta);
      }
   }

   void PrototypeTool::SetSelectionType(SelectionType selectionType)
   {
      mSelectionType = selectionType;
   }

   void PrototypeTool::RenderUi()
   {
      if (ImGui::CollapsingHeader("Prototype tool", ImGuiTreeNodeFlags_DefaultOpen))
      {
         ImGui::Text("Hello prototype tool!");
      }
   }
}