#include "PrototypeTool.h"
#include "core/components/Actor.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/CRigidBody.h"
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
      mPolyMesh = CreatePolyMeshCube();
      WriteToFile(mPolyMesh, "polymesh.obj");
      //gPhysics().EnableDebugDraw(true);
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

   void PrototypeTool::ActorSelected(Actor* actor, glm::vec3 normal)
   {
      CRenderable* renderable = actor->GetComponent<CRenderable>();
      mSelectedActor = actor;

      mSelectedModel = renderable->GetInternal()->GetModel();
      UpdateMeshBuffer(mSelectedModel->mMeshes[0], mPolyMesh);
   }

   static glm::vec3 toGlm(const OpenMesh::Vec3f& point)
   {
      glm::vec3 pos = glm::vec3(point[0], point[1], point[2]);

      return pos;
   }

   void PrototypeTool::Update(World* world, Actor* selectedActor)
   {
      if (mSelected)
      {
         glm::mat4 worldMatrix = mSelectedActor->GetTransform().GetWorldMatrix();
         std::vector<glm::vec3> quadVertices;
         for (auto vertex : mSelectedFace.vertices())
         {
            glm::vec3 pos = glm::vec3(mPolyMesh.point(vertex)[0],
                                      mPolyMesh.point(vertex)[1],
                                      mPolyMesh.point(vertex)[2]);

            pos = worldMatrix * glm::vec4(pos, 1.0f);

            quadVertices.push_back(pos);
         }

         // Face highlight
         Im3d::PushColor(Im3d::Color_Yellow);
         Im3d::DrawQuadFilled(quadVertices[0], quadVertices[1], quadVertices[2], quadVertices[3]);
         Im3d::PopColor();

         // Edge highlight
         auto toVh = mPolyMesh.to_vertex_handle(mSelectedEdge);
         auto fromVh = mPolyMesh.from_vertex_handle(mSelectedEdge);
         glm::vec3 toVertex = worldMatrix * glm::vec4(toGlm(mPolyMesh.point(toVh)), 1.0f);
         glm::vec3 fromVertex = worldMatrix * glm::vec4(toGlm(mPolyMesh.point(fromVh)), 1.0f);
         Im3d::DrawLine(toVertex, fromVertex, 6.0f, Im3d::Color_Green);
      }

      // Transform edge
      if (gInput().KeyPressed('N') || gInput().KeyPressed('M'))
      {
         auto normal = mPolyMesh.calc_normal(mSelectedFace);
         int operation = gInput().KeyPressed('N') ? 1 : -1;

         auto fromVh = mSelectedEdge.from();
         auto toVh = mSelectedEdge.to();

         mPolyMesh.set_point(fromVh, mPolyMesh.point(fromVh) + operation * OpenMesh::Vec3f(0.25f, 0.25f, 0.25f) * normal);
         mPolyMesh.set_point(toVh, mPolyMesh.point(toVh) + operation * OpenMesh::Vec3f(0.25f, 0.25f, 0.25f) * normal);

         UpdateMeshBuffer(mSelectedModel->mMeshes[0], mPolyMesh);
      }

      // Transform face
      if (gInput().KeyPressed('T') || gInput().KeyPressed('Y'))
      {
         auto normal = mPolyMesh.calc_normal(mSelectedFace);
         int operation = gInput().KeyPressed('T') ? 1 : -1;

         for (auto vertex : mSelectedFace.vertices())
         {
            mPolyMesh.set_point(vertex, mPolyMesh.point(vertex) + operation * OpenMesh::Vec3f(0.25f, 0.25f, 0.25f) * normal);
         }

         UpdateMeshBuffer(mSelectedModel->mMeshes[0], mPolyMesh);
      }

      // Scale face
      if (gInput().KeyPressed('F') || gInput().KeyPressed('G'))
      {
         auto center = mPolyMesh.calc_face_centroid(mSelectedFace);
         int operation = gInput().KeyPressed('G') ? 1 : -1;

         for (auto vertex : mSelectedFace.vertices())
         {
            auto delta = mPolyMesh.point(vertex) - center;
            mPolyMesh.set_point(vertex, mPolyMesh.point(vertex) + operation * delta * 0.2f);
         }

         UpdateMeshBuffer(mSelectedModel->mMeshes[0], mPolyMesh);
      }

      // Rotate face
      if (gInput().KeyPressed('J') || gInput().KeyPressed('K'))
      {
         auto center = mPolyMesh.calc_face_centroid(mSelectedFace);
         int operation = gInput().KeyPressed('J') ? 1 : -1;

         for (auto vertex : mSelectedFace.vertices())
         {
            auto delta = mPolyMesh.point(vertex) - center;

            if (delta[0] > 0.0f)
               mPolyMesh.set_point(vertex, mPolyMesh.point(vertex) + operation * OpenMesh::Vec3f(0.0f, 0.2f, 0.0f));
            if (delta[0] < 0.0f)
               mPolyMesh.set_point(vertex, mPolyMesh.point(vertex) - operation * OpenMesh::Vec3f(0.0f, 0.2f, 0.0f));
         }

         UpdateMeshBuffer(mSelectedModel->mMeshes[0], mPolyMesh);
      }

      // Add face
      if (gInput().KeyPressed('R'))
      {
         auto normal = mPolyMesh.calc_normal(mSelectedFace);
         std::vector<PolyMesh::VertexHandle> vhandles(8);
         auto vhIter = mSelectedFace.vertices().begin();

         // Bottom vertices shared with existing faces
         vhandles[0] = *(vhIter++);
         vhandles[1] = *(vhIter++);
         vhandles[5] = *(vhIter++);
         vhandles[4] = *vhIter;

         // New vertices for top face
         vhandles[2] = mPolyMesh.add_vertex(mPolyMesh.point(vhandles[1]) + normal * 1.0f);
         vhandles[3] = mPolyMesh.add_vertex(mPolyMesh.point(vhandles[0]) + normal * 1.0f);
         vhandles[6] = mPolyMesh.add_vertex(mPolyMesh.point(vhandles[5]) + normal * 1.0f);
         vhandles[7] = mPolyMesh.add_vertex(mPolyMesh.point(vhandles[4]) + normal * 1.0f);

         mPolyMesh.delete_face(mSelectedFace);
         mPolyMesh.garbage_collection();
         mSelectedFace = AddFaceExtrude(vhandles);

         UpdateMeshBuffer(mSelectedModel->mMeshes[0], mPolyMesh);
      }

      if (gInput().KeyPressed(VK_LBUTTON))
      {
         OpenMesh::SmartFaceHandle closestFace;
         float closestDistance = FLT_MAX;
         glm::vec3 closestIntersectionPoint;

         for (const auto& face : mPolyMesh.faces())
         {
            std::vector<glm::vec3> vertices;

            for (const auto& vertex : face.vertices())
            {
               glm::vec3 pos = glm::vec3(mPolyMesh.point(vertex)[0],
                                         mPolyMesh.point(vertex)[1],
                                         mPolyMesh.point(vertex)[2]);

               vertices.push_back(pos);
            }

            Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
            glm::mat4 inverseWorldMatrix = glm::inverse(mSelectedActor->GetTransform().GetWorldMatrix());
            ray.origin = inverseWorldMatrix * glm::vec4(ray.origin, 1.0f);
            ray.direction = inverseWorldMatrix * glm::vec4(ray.direction, 0.0f);

            float distance;
            glm::vec3 intersectPoint;
            if (ray.TriangleIntersect(vertices[0], vertices[2], vertices[1], intersectPoint, distance) ||
                ray.TriangleIntersect(vertices[0], vertices[3], vertices[2], intersectPoint, distance))
            {

               if (distance < closestDistance)
               {
                  closestFace = face;
                  closestDistance = distance;
                  closestIntersectionPoint = intersectPoint;
               }
            }
         }

         if (closestDistance != FLT_MAX)
         {
            auto n = mPolyMesh.calc_normal(closestFace);
            glm::vec3 normal = glm::vec3(n[0], n[1], n[2]);

            UTO_LOG("nx: " + std::to_string(normal.x) +
                  " ny: " + std::to_string(normal.y) +
                  " nz: " + std::to_string(normal.z) +
                  " distance: " + std::to_string(closestDistance));

            mSelectedFace = closestFace;
            mSelected = true;

            mSelectedEdge = GetClosestEdge(mSelectedFace, closestIntersectionPoint);
         }
         else
         {
            UTO_LOG("No hit!");
         }
      }
      
      if (gInput().KeyPressed('U'))
      {
         WriteToFile(mPolyMesh, "polymesh.obj");
      }
   }

   void PrototypeTool::RenderUi()
   {
      if (ImGui::CollapsingHeader("Prototype tool", ImGuiTreeNodeFlags_DefaultOpen))
      {
         ImGui::Text("Hello prototype tool!");
      }
   }

   OpenMesh::SmartHalfedgeHandle PrototypeTool::GetClosestEdge(const OpenMesh::SmartFaceHandle& face, glm::vec3 point)
   {
      float minDistance = FLT_MAX;
      OpenMesh::SmartHalfedgeHandle closestEdge;

      for (auto& halfEdge : face.halfedges())
      {
         auto toVertex = mPolyMesh.to_vertex_handle(halfEdge);
         auto fromVertex = mPolyMesh.from_vertex_handle(halfEdge);

         float distance = Math::DistanceToLine(toGlm(mPolyMesh.point(toVertex)), toGlm(mPolyMesh.point(fromVertex)), point);
         if (distance < minDistance)
         {
            minDistance = distance;
            closestEdge = halfEdge;
         }
      }

      return closestEdge;
   }

   void PrototypeTool::UpdateMeshBuffer(Utopian::Vk::Mesh* mesh, const PolyMesh& polyMesh)
   {
      mesh->vertexVector.clear();
      mesh->indexVector.clear();

      uint32_t faceOffset = 0u;

      for (const auto& face : polyMesh.faces())
      {
         auto n = polyMesh.calc_normal(face);
         glm::vec3 normal = glm::vec3(n[0], n[1], n[2]);

         std::vector<uint32_t> indices;

         uint32_t uvIndex = 0u;
         glm::vec2 texCoords[4] = {
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f)
         };

         for (const auto& vertex : face.vertices())
         {
            glm::vec3 pos = glm::vec3(polyMesh.point(vertex)[0],
                                      polyMesh.point(vertex)[1],
                                      polyMesh.point(vertex)[2]);

            glm::vec2 texCoord = texCoords[uvIndex];
            
            mesh->AddVertex(Vk::Vertex(-pos, -normal, texCoord));

            indices.push_back(faceOffset++);
            uvIndex++;
         }

         mesh->AddTriangle(indices[0], indices[1], indices[2]);
         mesh->AddTriangle(indices[0], indices[2], indices[3]);
      }

      mRebuildMeshBuffer = true;
   }

   PolyMesh PrototypeTool::CreatePolyMeshCube()
   {
      PolyMesh mesh;

      mesh.request_face_status();
      mesh.request_edge_status();
      mesh.request_vertex_status();

      // Generate vertices
      PolyMesh::VertexHandle vhandle[8];
      vhandle[0] = mesh.add_vertex(PolyMesh::Point(-0.5, -0.5f,  0.5f));
      vhandle[1] = mesh.add_vertex(PolyMesh::Point( 0.5f, -0.5f,  0.5f));
      vhandle[2] = mesh.add_vertex(PolyMesh::Point( 0.5f,  0.5f,  0.5f));
      vhandle[3] = mesh.add_vertex(PolyMesh::Point(-0.5f,  0.5f,  0.5f));
      vhandle[4] = mesh.add_vertex(PolyMesh::Point(-0.5f, -0.5f, -0.5f));
      vhandle[5] = mesh.add_vertex(PolyMesh::Point( 0.5f, -0.5f, -0.5f));
      vhandle[6] = mesh.add_vertex(PolyMesh::Point( 0.5f,  0.5f, -0.5f));
      vhandle[7] = mesh.add_vertex(PolyMesh::Point(-0.5f,  0.5f, -0.5f));

      // Generate (quadrilateral) faces
      std::vector<PolyMesh::VertexHandle> vertexHandles;
      vertexHandles.clear();
      vertexHandles.push_back(vhandle[0]);
      vertexHandles.push_back(vhandle[1]);
      vertexHandles.push_back(vhandle[2]);
      vertexHandles.push_back(vhandle[3]);
      mesh.add_face(vertexHandles);
      
      vertexHandles.clear();
      vertexHandles.push_back(vhandle[7]);
      vertexHandles.push_back(vhandle[6]);
      vertexHandles.push_back(vhandle[5]);
      vertexHandles.push_back(vhandle[4]);
      mesh.add_face(vertexHandles);

      vertexHandles.clear();
      vertexHandles.push_back(vhandle[1]);
      vertexHandles.push_back(vhandle[0]);
      vertexHandles.push_back(vhandle[4]);
      vertexHandles.push_back(vhandle[5]);
      mesh.add_face(vertexHandles);

      vertexHandles.clear();
      vertexHandles.push_back(vhandle[2]);
      vertexHandles.push_back(vhandle[1]);
      vertexHandles.push_back(vhandle[5]);
      vertexHandles.push_back(vhandle[6]);
      mesh.add_face(vertexHandles);

      vertexHandles.clear();
      vertexHandles.push_back(vhandle[3]);
      vertexHandles.push_back(vhandle[2]);
      vertexHandles.push_back(vhandle[6]);
      vertexHandles.push_back(vhandle[7]);
      mesh.add_face(vertexHandles);

      vertexHandles.clear();
      vertexHandles.push_back(vhandle[0]);
      vertexHandles.push_back(vhandle[3]);
      vertexHandles.push_back(vhandle[7]);
      vertexHandles.push_back(vhandle[4]);
      mesh.add_face(vertexHandles);

      return mesh;
   }

   OpenMesh::SmartFaceHandle PrototypeTool::AddFaceExtrude(std::vector<PolyMesh::VertexHandle>& vhandle)
   {
      // Generate (quadrilateral) faces
      std::vector<PolyMesh::VertexHandle> vertexHandles;
      vertexHandles.clear();
      vertexHandles.push_back(vhandle[0]);
      vertexHandles.push_back(vhandle[1]);
      vertexHandles.push_back(vhandle[2]);
      vertexHandles.push_back(vhandle[3]);
      auto fh = mPolyMesh.add_face(vertexHandles);
      
      vertexHandles.clear();
      vertexHandles.push_back(vhandle[7]);
      vertexHandles.push_back(vhandle[6]);
      vertexHandles.push_back(vhandle[5]);
      vertexHandles.push_back(vhandle[4]);
      mPolyMesh.add_face(vertexHandles);

      // vertexHandles.clear();
      // vertexHandles.push_back(vhandle[1]);
      // vertexHandles.push_back(vhandle[0]);
      // vertexHandles.push_back(vhandle[4]);
      // vertexHandles.push_back(vhandle[5]);
      // mPolyMesh.add_face(vertexHandles);

      vertexHandles.clear();
      vertexHandles.push_back(vhandle[2]);
      vertexHandles.push_back(vhandle[1]);
      vertexHandles.push_back(vhandle[5]);
      vertexHandles.push_back(vhandle[6]);
      mPolyMesh.add_face(vertexHandles);

      vertexHandles.clear();
      vertexHandles.push_back(vhandle[3]);
      vertexHandles.push_back(vhandle[2]);
      vertexHandles.push_back(vhandle[6]);
      vertexHandles.push_back(vhandle[7]);
      OpenMesh::SmartFaceHandle topFace = mPolyMesh.add_face(vertexHandles);

      vertexHandles.clear();
      vertexHandles.push_back(vhandle[0]);
      vertexHandles.push_back(vhandle[3]);
      vertexHandles.push_back(vhandle[7]);
      vertexHandles.push_back(vhandle[4]);
      mPolyMesh.add_face(vertexHandles);

      return topFace;
   }
  
   void PrototypeTool::WriteToFile(const PolyMesh& mesh, std::string file)
   {
      UTO_LOG("vertices: " + std::to_string(mesh.n_vertices()));
      UTO_LOG("half edges: " + std::to_string(mesh.n_halfedges()));
      UTO_LOG("edges: " + std::to_string(mesh.n_edges()));
      UTO_LOG("faces: " + std::to_string(mesh.n_faces()));

      if (!OpenMesh::IO::write_mesh(mesh, file))
      {
         std::cerr << "Cannot write mesh to file " << file << std::endl;
      }
   }
}