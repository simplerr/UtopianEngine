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
      // Transform edge
      if (gInput().KeyPressed('N') || gInput().KeyPressed('M'))
      {
         glm::vec3 normal = mSelectedMesh->GetSelectedFaceNormal();
         float operation = gInput().KeyPressed('N') ? 1 : -1;

         mSelectedMesh->MoveSelectedEdge(normal * operation * 0.25f);
      }

      // Transform face
      if (gInput().KeyPressed('T') || gInput().KeyPressed('Y'))
      {
         glm::vec3 normal = mSelectedMesh->GetSelectedFaceNormal();
         float operation = gInput().KeyPressed('T') ? 1 : -1;

         mSelectedMesh->MoveSelectedFace(normal * operation * 0.25f);
      }

      // Scale face
      if (gInput().KeyPressed('F') || gInput().KeyPressed('G'))
      {
         float operation = gInput().KeyPressed('F') ? 1 : -1;

         mSelectedMesh->ScaleSelectedFace(operation * 0.2f);
      }

      // Add face
      if (gInput().KeyPressed('R'))
      {
         mSelectedMesh->ExtrudeSelectedFace(1.0f);
      }

      if (gInput().KeyPressed(VK_LBUTTON))
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
          glm::vec3 v0, v1, v2, v3;
          mSelectedMesh->GetSelectedFaceVertices(v0, v1, v2, v3);

          // Face highlight
          Im3d::PushColor(Im3d::Color_Yellow);
          Im3d::DrawQuadFilled(v0, v1, v2, v3);
          Im3d::PopColor();

          // Edge highlight
          mSelectedMesh->GetSelectedEdgeVertices(v0, v1);
          Im3d::DrawLine(v0, v1, 6.0f, Im3d::Color_Green);
      }

   }

   void PrototypeTool::RenderUi()
   {
      if (ImGui::CollapsingHeader("Prototype tool", ImGuiTreeNodeFlags_DefaultOpen))
      {
         ImGui::Text("Hello prototype tool!");
      }
   }
}