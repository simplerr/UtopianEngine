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
      //AddPolymesh(glm::vec3(0.0f, 0.5f, 0.0f), "data/textures/prototype/Orange/texture_01.ktx");
      //AddPolymesh(glm::vec3(3.0f, 0.5f, 0.0f), "data/textures/prototype/Orange/texture_01.ktx");
      //AddPolymesh(glm::vec3(7.5f, 0.5f, 7.5f), "data/textures/prototype/Green/texture_01.png");
   }

   PrototypeTool::~PrototypeTool()
   {

   }

   void PrototypeTool::PreFrame()
   {
      if (mSelectedMesh != nullptr)
      {
         mSelectedMesh->PreFrame();
      }
   }

   void PrototypeTool::ActorSelected(Actor* actor)
   {
      if (actor != nullptr && actor->HasComponent<CPolyMesh>())
      {
         mSelectedActor = actor;
         mSelectedMesh = actor->GetComponent<CPolyMesh>();
      }
      else
      {
         mSelectedActor = nullptr;
         mSelectedMesh = nullptr;
      }
   }

   void PrototypeTool::Update(World* world)
   {
      // Add polymesh to level
      if (gInput().KeyPressed('X') && gInput().KeyDown(VK_LCONTROL))
      {
         Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
         IntersectionInfo intersectInfo = gWorld().RayIntersection(ray);
         glm::vec3 position = ray.origin + ray.direction * intersectInfo.distance;
         position += glm::vec3(0.0f, 0.5f, 0.0f);
         AddPolymesh(position, "data/textures/prototype/Orange/texture_01.ktx");
      }

      if (mSelectedMesh != nullptr)
      {
         // Select mesh face
         if (gInput().KeyPressed(VK_LBUTTON) && gInput().KeyDown(VK_LCONTROL))
         {
            Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
            mSelectedMesh->SelectFace(ray);
         }

         if (mSelectionType == FACE_SELECTION)
            DrawFaceGizmo();
         else if (mSelectionType == EDGE_SELECTION)
            DrawEdgeGizmo();
      }
   }

   void PrototypeTool::AddPolymesh(glm::vec3 position, std::string texture)
   {
      SharedPtr<Utopian::Actor> actor = Utopian::Actor::Create("Polymesh");
      Utopian::CTransform* transform = actor->AddComponent<Utopian::CTransform>(position);
      Utopian::CRenderable* renderable = actor->AddComponent<Utopian::CRenderable>();
      Utopian::CRigidBody* rigidBody = actor->AddComponent<Utopian::CRigidBody>();
      Utopian::CPolyMesh* polyMesh = actor->AddComponent<Utopian::CPolyMesh>();

      auto model = Utopian::Vk::gModelLoader().LoadBox(texture);
      renderable->SetModel(model);

      rigidBody->SetCollisionShapeType(Utopian::CollisionShapeType::MESH);
      
      actor->PostInit();
      Utopian::World::Instance().SynchronizeNodeTransforms();

      // Must be called after PostInit() since it needs the Renderable component
      rigidBody->SetKinematic(true);
      rigidBody->SetFriction(0.5f);
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

      static bool alreadyExtruded = false;
      static bool meshModified = false;

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

         // Extrude face
         if (gInput().KeyDown(VK_SHIFT) && !alreadyExtruded)
         {
            mSelectedMesh->ExtrudeSelectedFace(0.0f);
            alreadyExtruded = true;
         }

         meshModified = true;
      }
      else
      {
         alreadyExtruded = false;
         prevScale = glm::vec3(1.0f);

         if (meshModified)
         {
            UpdateRigidBody();
            meshModified = false;
         }
      }
   }

   void PrototypeTool::DrawEdgeGizmo()
   {
      // Edge highlight
      glm::vec3 v0, v1;
      mSelectedMesh->GetSelectedEdgeVertices(v0, v1);
      Im3d::DrawLine(v0, v1, 5.0f, Im3d::Color_Green);

      static bool meshModified = false;

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

         meshModified = true;
      }
      else
      {
         if (meshModified)
         {
            UpdateRigidBody();
            meshModified = false;
         }
      }
   }

   void PrototypeTool::UpdateRigidBody()
   {
      CRigidBody* rigidBody = mSelectedActor->GetComponent<CRigidBody>();
      rigidBody->AddToWorld();
   }

   void PrototypeTool::SetSelectionType(SelectionType selectionType)
   {
      mSelectionType = selectionType;
   }

   void PrototypeTool::RenderUi()
   {
      if (ImGui::CollapsingHeader("Prototype tool", ImGuiTreeNodeFlags_DefaultOpen))
      {
         ImGui::Text("Empty");
      }
   }
}