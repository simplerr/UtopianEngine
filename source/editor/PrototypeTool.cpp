#include "PrototypeTool.h"
#include "core/components/Actor.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/CRigidBody.h"
#include "core/Input.h"
#include "core/Log.h"
#include "core/renderer/Renderer.h"
#include "core/renderer/ImGuiRenderer.h"
#include "core/Camera.h"
#include "core/World.h"
#include "utility/math/Ray.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/StaticModel.h"
#include <vulkan/VulkanPrerequisites.h>

namespace Utopian
{
   PrototypeTool::PrototypeTool()
   {

   }

   PrototypeTool::~PrototypeTool()
   {

   }

   void PrototypeTool::PreFrame()
   {
      if (mSelectedModel != nullptr && mRebuildMeshBuffer)
      {
         mSelectedModel->mMeshes[0]->BuildBuffers(gRenderer().GetDevice());
         mSelectedModel->Init(gRenderer().GetDevice());
         mRebuildMeshBuffer = false;
      }
   }

   void PrototypeTool::ActorSelected(Actor* actor, glm::vec3 normal)
   {
         CRenderable* renderable = actor->GetComponent<CRenderable>();

         mSelectedModel = renderable->GetInternal()->GetModel();
         mSelectedFaceNormal = normal;

         // UTO_LOG("nx: " + std::to_string(normal.x) +
         //         " ny: " + std::to_string(normal.y) +
         //         " nz: " + std::to_string(normal.z));
   }

   void PrototypeTool::Update(World* world, Actor* selectedActor)
   {
      //mSelectedFaceNormal = glm::vec3(-1.0f, 0.0f, 0.0f);

      if (gInput().KeyPressed('T'))
      {
         Utopian::Vk::Mesh* mesh = mSelectedModel->mMeshes[0];

         for (Utopian::Vk::Vertex& vertex : mesh->vertexVector)
         {
            // Right
            if (mSelectedFaceNormal == glm::vec3(-1.0f, 0.0f, 0.0f))
            {
               if (vertex.Pos.x > 0.0f)
               {
                  vertex.Pos.x += 1.0f;

                  if (vertex.Normal != glm::vec3(-1.0f, 0.0f, 0.0f))
                     vertex.Tex.x += 1.0f;
               }
            }

            // Left
            if (mSelectedFaceNormal == glm::vec3(1.0f, 0.0f, 0.0f))
            {
               if (vertex.Pos.x < 0.0f)
               {
                  vertex.Pos.x -= 1.0f;

                  if (vertex.Normal != glm::vec3(1.0f, 0.0f, 0.0f))
                     vertex.Tex.x -= 1.0f;
               }
            }

            // Top
            if (mSelectedFaceNormal == glm::vec3(0.0f, 1.0f, 0.0f))
            {
               if (vertex.Pos.y < 0.0f)
               {
                  vertex.Pos.y -= 1.0f;

                  // Front & Back
                  if (glm::abs(vertex.Normal) == glm::vec3(0.0f, 0.0f, 1.0f))
                     vertex.Tex.y -= 1.0f;

                  // Left & Right
                  if (glm::abs(vertex.Normal) == glm::vec3(1.0f, 0.0f, 0.0f))
                     vertex.Tex.x -= 1.0f;
               }
            }

            // Bottom
            if (mSelectedFaceNormal == glm::vec3(0.0f, -1.0f, 0.0f))
            {
               if (vertex.Pos.y > 0.0f)
               {
                  vertex.Pos.y += 1.0f;

                  // Front & Back
                  if (glm::abs(vertex.Normal) == glm::vec3(0.0f, 0.0f, 1.0f))
                     vertex.Tex.y += 1.0f;

                  // Left & Right
                  if (glm::abs(vertex.Normal) == glm::vec3(1.0f, 0.0f, 0.0f))
                     vertex.Tex.x += 1.0f;
               }
            }
         }

         mRebuildMeshBuffer = true;
      }

      if (gInput().KeyPressed('Y'))
      {
         Utopian::Vk::Mesh* mesh = mSelectedModel->mMeshes[0];

         for (Utopian::Vk::Vertex& vertex : mesh->vertexVector)
         {
             if (vertex.Pos.x > 0.0f)
             {
                 vertex.Pos.y *= 1.2;
                 vertex.Pos.z *= 1.2;
                 vertex.Tex *= 1.2;
             }
         }

         mRebuildMeshBuffer = true;
      }


      if (gInput().KeyPressed('E'))
         mLastSelectedActor = nullptr;

      if (gInput().KeyReleased('E'))
         mNewlyAddedActors.clear();

      if (gInput().KeyDown('E'))
      {
         Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
         IntersectionInfo intersectInfo = world->RayIntersection(ray);

         if (intersectInfo.actor != nullptr)// && selectedActor.get() != mLastSelectedActor)
         {
            // UTO_LOG("nx: " + std::to_string(intersectInfo.normal.x) +
            //         " ny: " + std::to_string(intersectInfo.normal.y) +
            //         " nz: " + std::to_string(intersectInfo.normal.z));

            const float epsilon = 0.05f;
            glm::vec3 pos = ray.origin + ray.direction * intersectInfo.distance + intersectInfo.normal * epsilon;
            float offsetx = pos.x >= 0 ? 0.5 : -0.5f;
            float offsety = pos.y >= 0 ? 0.5 : -0.5f;
            float offsetz = pos.z >= 0 ? 0.5 : -0.5f;
            pos = glm::vec3((int)pos.x + offsetx, (int)pos.y + offsety, (int)pos.z + offsetz);

            if (pos != mLastAddedPosition)
            {
               bool newActorIntersection = false;
               for (auto& actor : mNewlyAddedActors)
               {
                  if (actor == intersectInfo.actor.get())
                  {
                     newActorIntersection = true;
                     break;
                  }
               }

               if (!newActorIntersection)
               {
                  UTO_LOG("Add!");

                  Actor* box = AddBox(pos, "data/textures/prototype/Orange/texture_01.ktx");
                  mNewlyAddedActors.push_back(box);

                  mLastAddedPosition = pos;
               }
            }

            //mLastSelectedActor = intersectInfo.actor.get();
         }
      }

      if (gInput().KeyDown('X'))
      {
         Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
         IntersectionInfo intersectInfo = world->RayIntersection(ray, PrototypeBoxSceneLayer);

         if (intersectInfo.actor != nullptr)// && selectedActor.get() != mLastSelectedActor)
         {
            intersectInfo.actor->SetAlive(false);
         }
      }
   }

   void PrototypeTool::RenderUi()
   {
      if (ImGui::CollapsingHeader("Prototype tool", ImGuiTreeNodeFlags_DefaultOpen))
      {
         ImGui::Text("Hello prototype tool!");
      }
   }

   Actor* PrototypeTool::AddBox(glm::vec3 position, std::string texture)
   {
      SharedPtr<Utopian::Actor> actor = Utopian::Actor::Create("Box");
      Utopian::CTransform* transform = actor->AddComponent<Utopian::CTransform>(position);
      Utopian::CRenderable* renderable = actor->AddComponent<Utopian::CRenderable>();
      Utopian::CRigidBody* rigidBody = actor->AddComponent<Utopian::CRigidBody>();

      actor->SetSceneLayer(PrototypeBoxSceneLayer);

      auto model = Utopian::Vk::gModelLoader().LoadBox(texture);
      renderable->SetModel(model);

      rigidBody->SetCollisionShapeType(Utopian::CollisionShapeType::MESH);

      actor->PostInit();
      Utopian::World::Instance().SynchronizeNodeTransforms();

      // Must be called after PostInit() since it needs the Renderable component
      rigidBody->SetKinematic(true);

      return actor.get();
   }
}