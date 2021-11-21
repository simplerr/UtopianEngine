#include "core/components/CPolyMesh.h"
#include "core/components/Actor.h"
#include "core/Log.h"
#include "core/LuaManager.h"
#include "core/renderer/Renderer.h"
#include <OpenMesh/Core/Mesh/Handles.hh>
#include <OpenMesh/Core/Mesh/PolyConnectivity.hh>
#include <core/components/CRenderable.h>
#include <core/components/CRigidBody.h>
#include <core/components/Component.h>
#include <core/renderer/Model.h>
#include "OpenMesh/Core/IO/MeshIO.hh"
#include "vulkan/TextureLoader.h"
#include "utility/math/Helpers.h"

namespace Utopian
{
   static glm::vec3 ToGlm(const OpenMesh::Vec3f& point)
   {
      glm::vec3 pos = glm::vec3(point[0], point[1], point[2]);

      return pos;
   }

   CPolyMesh::CPolyMesh(Actor* parent)
      : Component(parent)
   {
      SetName("CPolyMesh");

      uint64_t id = Math::GetRandom();
      std::string name = "polymesh-" + std::to_string(id) + ".obj";

      SetModelPath("data/models/polymesh/" + name);
      SetTexturePath("data/textures/prototype/Orange/texture_01.ktx");

      mPolyMesh.request_face_status();
      mPolyMesh.request_edge_status();
      mPolyMesh.request_vertex_status();

      CreateCube();
   }
   
   CPolyMesh::CPolyMesh(Actor* parent, std::string modelPath, std::string texturePath)
      : Component(parent)
   {
      SetName("CPolyMesh");

      SetModelPath(modelPath);
      SetTexturePath(texturePath);

      mPolyMesh.request_face_status();
      mPolyMesh.request_edge_status();
      mPolyMesh.request_vertex_status();

      LoadFromFile(modelPath);
   }

   CPolyMesh::~CPolyMesh()
   {
   }

   void CPolyMesh::Update(double deltaTime)
   {
      if (mFirstFrame)
      {
         // Update rigid body collision shape
         // Note: Should be in PostInit() but the CRigidBodys PostInit is not 
         // guaranteed to been called before CPolyMesh::PostInit
         CRigidBody* rigidBody = GetParent()->GetComponent<CRigidBody>();
         rigidBody->AddToWorld();
         mFirstFrame = false;
      }
   }

   void CPolyMesh::OnCreated()
   {
   }

   void CPolyMesh::OnDestroyed()
   {
   }

   void CPolyMesh::PostInit()
   {
      CRenderable* renderable = GetParent()->GetComponent<CRenderable>();
      renderable->SetDiffuseTexture(0, Vk::gTextureLoader().LoadTexture(mTexturePath));

      UpdateMeshBuffer();
   }

   void CPolyMesh::SerializeData()
   {
      WriteToFile(GetModelPath());
   }

   void CPolyMesh::PreFrame()
   {
      if (mRebuildMeshBuffer)
      {
         UpdateMeshBuffer();
         mRebuildMeshBuffer = false;
      }
   }

   LuaPlus::LuaObject CPolyMesh::GetLuaObject()
   {
      LuaPlus::LuaObject luaObject;
      luaObject.AssignNewTable(gLuaManager().GetLuaState());

      luaObject.SetString("modelPath", mModelPath.c_str());
      luaObject.SetString("texturePath", mTexturePath.c_str());

      return luaObject;
   }

   void CPolyMesh::GetSelectedFaceVertices(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2, glm::vec3& v3)
   {
      glm::mat4 worldMatrix = GetParent()->GetTransform().GetWorldMatrix();

      std::vector<glm::vec3> vertices;
      for (auto vertex : mSelectedFace.vertices())
      {
         glm::vec3 pos = ToGlm(mPolyMesh.point(vertex));
         pos = worldMatrix * glm::vec4(pos, 1.0f);

         vertices.push_back(pos);
      }

      v0 = vertices[0];
      v1 = vertices[1];
      v2 = vertices[2];
      v3 = vertices[3];
   }

   void CPolyMesh::GetSelectedEdgeVertices(glm::vec3& v0, glm::vec3& v1)
   {
      glm::mat4 worldMatrix = GetParent()->GetTransform().GetWorldMatrix();

      OpenMesh::VertexHandle toVh = mPolyMesh.to_vertex_handle(mSelectedEdge);
      OpenMesh::VertexHandle fromVh = mPolyMesh.from_vertex_handle(mSelectedEdge);
      glm::vec3 toVertex = worldMatrix * glm::vec4(ToGlm(mPolyMesh.point(toVh)), 1.0f);
      glm::vec3 fromVertex = worldMatrix * glm::vec4(ToGlm(mPolyMesh.point(fromVh)), 1.0f);

      v0 = toVertex;
      v1 = fromVertex;
   }

   glm::vec3 CPolyMesh::GetSelectedFaceNormal()
   {
      OpenMesh::Vec3f n = mPolyMesh.calc_normal(mSelectedFace);
      glm::vec3 normal = ToGlm(n);

      return normal;
   }

   glm::vec3 CPolyMesh::GetSelectedFaceCenter()
   {
      OpenMesh::Vec3f c = mPolyMesh.calc_face_centroid(mSelectedFace);
      glm::vec3 center = ToGlm(c);

      glm::mat4 worldMatrix = GetParent()->GetTransform().GetWorldMatrix();
      center = worldMatrix * glm::vec4(center, 1.0f);

      return center;
   }
   
   void CPolyMesh::MoveSelectedEdge(const glm::vec3& delta)
   {
      auto fromVh = mSelectedEdge.from();
      auto toVh = mSelectedEdge.to();

      mPolyMesh.set_point(fromVh, mPolyMesh.point(fromVh) + OpenMesh::Vec3f(delta.x, delta.y, delta.z));
      mPolyMesh.set_point(toVh, mPolyMesh.point(toVh) + OpenMesh::Vec3f(delta.x, delta.y, delta.z));

      mRebuildMeshBuffer = true;
   }

   void CPolyMesh::MoveSelectedFace(const glm::vec3& delta)
   {
      for (auto vertex : mSelectedFace.vertices())
      {
         mPolyMesh.set_point(vertex, mPolyMesh.point(vertex) + OpenMesh::Vec3f(delta.x, delta.y, delta.z));
      }

      mRebuildMeshBuffer = true;
   }

   void CPolyMesh::ScaleSelectedFace(float delta)
   {
      OpenMesh::Vec3f center = mPolyMesh.calc_face_centroid(mSelectedFace);

      for (auto vertex : mSelectedFace.vertices())
      {
         OpenMesh::Vec3f diff = mPolyMesh.point(vertex) - center;
         mPolyMesh.set_point(vertex, mPolyMesh.point(vertex) + diff * delta);
      }

      mRebuildMeshBuffer = true;
   }

   void CPolyMesh::ExtrudeSelectedFace(float extrusion)
   {
      std::vector<PolyMesh::VertexHandle> vertices;
      vertices = AddExtrusionVertices(extrusion);

      mPolyMesh.delete_face(mSelectedFace);
      mPolyMesh.garbage_collection();
      mSelectedFace = AddExtrusionFaces(vertices);

      mRebuildMeshBuffer = true;
   }

   void CPolyMesh::SelectFace(Ray ray)
   {
      OpenMesh::SmartFaceHandle closestFace;
      float closestDistance = FLT_MAX;
      glm::vec3 closestIntersectionPoint;

      for (const auto& face : mPolyMesh.faces())
      {
         std::vector<glm::vec3> vertices;

         for (const auto& vertex : face.vertices())
         {
            glm::vec3 pos = ToGlm(mPolyMesh.point(vertex));
            vertices.push_back(pos);
         }

         Ray localRay = ray;
         glm::mat4 inverseWorldMatrix = glm::inverse(GetParent()->GetTransform().GetWorldMatrix());
         localRay.origin = inverseWorldMatrix * glm::vec4(localRay.origin, 1.0f);
         localRay.direction = inverseWorldMatrix * glm::vec4(localRay.direction, 0.0f);

         float distance;
         glm::vec3 intersectPoint;
         if (localRay.TriangleIntersect(vertices[0], vertices[2], vertices[1], intersectPoint, distance) ||
             localRay.TriangleIntersect(vertices[0], vertices[3], vertices[2], intersectPoint, distance))
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

         // UTO_LOG("nx: " + std::to_string(normal.x) +
         //       " ny: " + std::to_string(normal.y) +
         //       " nz: " + std::to_string(normal.z) +
         //       " distance: " + std::to_string(closestDistance));

         mSelectedFace = closestFace;

         mSelectedEdge = GetClosestEdge(mSelectedFace, closestIntersectionPoint);
      }
   }

   OpenMesh::SmartHalfedgeHandle CPolyMesh::GetClosestEdge(const OpenMesh::SmartFaceHandle& face, glm::vec3 point)
   {
      float minDistance = FLT_MAX;
      OpenMesh::SmartHalfedgeHandle closestEdge;

      for (auto& halfEdge : face.halfedges())
      {
         auto toVertex = mPolyMesh.to_vertex_handle(halfEdge);
         auto fromVertex = mPolyMesh.from_vertex_handle(halfEdge);

         float distance = Math::DistanceToLine(ToGlm(mPolyMesh.point(toVertex)),
                                               ToGlm(mPolyMesh.point(fromVertex)), point);
         if (distance < minDistance)
         {
            minDistance = distance;
            closestEdge = halfEdge;
         }
      }

      return closestEdge;
   }

   void CPolyMesh::UpdateMeshBuffer()
   {
      CRenderable* renderable = GetParent()->GetComponent<CRenderable>();
      Model* model = renderable->GetInternal()->GetModel();
      Primitive* primitive = model->GetPrimitive(0);

      primitive->vertices.clear();
      primitive->indices.clear();

      uint32_t faceOffset = 0u;

      for (const auto& face : mPolyMesh.faces())
      {
         auto n = mPolyMesh.calc_normal(face);
         glm::vec3 normal = glm::vec3(n[0], n[1], n[2]);
         
         // Calculate tangent and bitangent vectors from two arbitrary vertices on the plane
         glm::vec3 v1 = ToGlm(mPolyMesh.point(face.vertices().begin()));
         glm::vec3 v2 = ToGlm(mPolyMesh.point(++face.vertices().begin()));
         glm::vec3 tangent = glm::vec3(glm::normalize(v1 - v2));
         glm::vec3 bitangent = glm::cross(normal, tangent);

         // If no texture tiling is desired:
         // glm::vec2 texCoords[4] = {
         //    glm::vec2(0.0f, 0.0f),
         //    glm::vec2(1.0f, 0.0f),
         //    glm::vec2(1.0f, 1.0f),
         //    glm::vec2(0.0f, 1.0f)
         // };

         std::vector<uint32_t> indices;
         uint32_t uvIndex = 0u;

         for (const auto& vertex : face.vertices())
         {
            glm::vec3 pos = ToGlm(mPolyMesh.point(vertex));
            glm::vec2 texCoord;

            // Use vertex position as texture coordinates from the
            // axis the face is the most aligned with
            float maxResult = FLT_MIN;
            float dotResult = abs(glm::dot(normal, glm::vec3(1.0f, 0.0f, 0.0f)));
            if (dotResult > maxResult) {
               texCoord = glm::vec2(pos.z, pos.y);
               maxResult = dotResult;
            }

            dotResult = abs(glm::dot(normal, glm::vec3(0.0f, 0.0f, 1.0f)));
            if (dotResult > maxResult) {
               texCoord = glm::vec2(pos.x, pos.y);
               maxResult = dotResult;
            }

            dotResult = abs(glm::dot(normal, glm::vec3(0.0f, 1.0f, 0.0f)));
            if (dotResult > maxResult) {
               texCoord = glm::vec2(pos.x, pos.z);
               maxResult = dotResult;
            }
            
            primitive->AddVertex(Vk::Vertex(-pos, -normal, texCoord, tangent, bitangent));

            indices.push_back(faceOffset++);
            uvIndex++;
         }

         primitive->AddTriangle(indices[0], indices[1], indices[2]);
         primitive->AddTriangle(indices[0], indices[2], indices[3]);
      }

      primitive->BuildBuffers(gRenderer().GetDevice());
      model->Init();
   }

   std::vector<PolyMesh::VertexHandle> CPolyMesh::AddExtrusionVertices(float extrusion)
   {
      auto normal = mPolyMesh.calc_normal(mSelectedFace);
      std::vector<PolyMesh::VertexHandle> vertices(8);
      auto vhIter = mSelectedFace.vertices().begin();

      // Bottom vertices shared with existing faces
      vertices[0] = *(vhIter++);
      vertices[1] = *(vhIter++);
      vertices[5] = *(vhIter++);
      vertices[4] = *vhIter;

      // New vertices for top face
      vertices[2] = mPolyMesh.add_vertex(mPolyMesh.point(vertices[1]) + normal * extrusion);
      vertices[3] = mPolyMesh.add_vertex(mPolyMesh.point(vertices[0]) + normal * extrusion);
      vertices[6] = mPolyMesh.add_vertex(mPolyMesh.point(vertices[5]) + normal * extrusion);
      vertices[7] = mPolyMesh.add_vertex(mPolyMesh.point(vertices[4]) + normal * extrusion);

      return vertices;
   }

   OpenMesh::SmartFaceHandle CPolyMesh::AddExtrusionFaces(std::vector<PolyMesh::VertexHandle>& vertices)
   {
      // Generate (quadrilateral) faces
      std::vector<PolyMesh::VertexHandle> vertexHandles;
      vertexHandles.clear();
      vertexHandles.push_back(vertices[0]);
      vertexHandles.push_back(vertices[1]);
      vertexHandles.push_back(vertices[2]);
      vertexHandles.push_back(vertices[3]);
      auto fh = mPolyMesh.add_face(vertexHandles);
      
      vertexHandles.clear();
      vertexHandles.push_back(vertices[7]);
      vertexHandles.push_back(vertices[6]);
      vertexHandles.push_back(vertices[5]);
      vertexHandles.push_back(vertices[4]);
      mPolyMesh.add_face(vertexHandles);

      vertexHandles.clear();
      vertexHandles.push_back(vertices[2]);
      vertexHandles.push_back(vertices[1]);
      vertexHandles.push_back(vertices[5]);
      vertexHandles.push_back(vertices[6]);
      mPolyMesh.add_face(vertexHandles);

      vertexHandles.clear();
      vertexHandles.push_back(vertices[3]);
      vertexHandles.push_back(vertices[2]);
      vertexHandles.push_back(vertices[6]);
      vertexHandles.push_back(vertices[7]);
      OpenMesh::SmartFaceHandle topFace = mPolyMesh.add_face(vertexHandles);

      vertexHandles.clear();
      vertexHandles.push_back(vertices[0]);
      vertexHandles.push_back(vertices[3]);
      vertexHandles.push_back(vertices[7]);
      vertexHandles.push_back(vertices[4]);
      mPolyMesh.add_face(vertexHandles);

      return topFace;
   }

   void CPolyMesh::CreateCube()
   {
      // Generate vertices
      PolyMesh::VertexHandle vhandle[8];
      vhandle[0] = mPolyMesh.add_vertex(PolyMesh::Point(-0.5, -0.5f,  0.5f));
      vhandle[1] = mPolyMesh.add_vertex(PolyMesh::Point( 0.5f, -0.5f,  0.5f));
      vhandle[2] = mPolyMesh.add_vertex(PolyMesh::Point( 0.5f,  0.5f,  0.5f));
      vhandle[3] = mPolyMesh.add_vertex(PolyMesh::Point(-0.5f,  0.5f,  0.5f));
      vhandle[4] = mPolyMesh.add_vertex(PolyMesh::Point(-0.5f, -0.5f, -0.5f));
      vhandle[5] = mPolyMesh.add_vertex(PolyMesh::Point( 0.5f, -0.5f, -0.5f));
      vhandle[6] = mPolyMesh.add_vertex(PolyMesh::Point( 0.5f,  0.5f, -0.5f));
      vhandle[7] = mPolyMesh.add_vertex(PolyMesh::Point(-0.5f,  0.5f, -0.5f));

      // Generate (quadrilateral) faces
      std::vector<PolyMesh::VertexHandle> vertexHandles;
      vertexHandles.clear();
      vertexHandles.push_back(vhandle[0]);
      vertexHandles.push_back(vhandle[1]);
      vertexHandles.push_back(vhandle[2]);
      vertexHandles.push_back(vhandle[3]);
      mPolyMesh.add_face(vertexHandles);

      vertexHandles.clear();
      vertexHandles.push_back(vhandle[7]);
      vertexHandles.push_back(vhandle[6]);
      vertexHandles.push_back(vhandle[5]);
      vertexHandles.push_back(vhandle[4]);
      mPolyMesh.add_face(vertexHandles);

      vertexHandles.clear();
      vertexHandles.push_back(vhandle[1]);
      vertexHandles.push_back(vhandle[0]);
      vertexHandles.push_back(vhandle[4]);
      vertexHandles.push_back(vhandle[5]);
      mPolyMesh.add_face(vertexHandles);

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
      mPolyMesh.add_face(vertexHandles);

      vertexHandles.clear();
      vertexHandles.push_back(vhandle[0]);
      vertexHandles.push_back(vhandle[3]);
      vertexHandles.push_back(vhandle[7]);
      vertexHandles.push_back(vhandle[4]);
      mPolyMesh.add_face(vertexHandles);
   }

   void CPolyMesh::WriteToFile(std::string file)
   {
      if (!OpenMesh::IO::write_mesh(mPolyMesh, file))
      {
         UTO_LOG("Cannot write mesh to file " + file);
      }
   }

   void CPolyMesh::LoadFromFile(std::string file)
   {
      OpenMesh::IO::read_mesh(mPolyMesh, file);
      mRebuildMeshBuffer = true;
   }

   std::string CPolyMesh::GetModelPath() const
   {
      return mModelPath;
   }

   std::string CPolyMesh::GetTexturePath() const
   {
      return mTexturePath;
   }

   void CPolyMesh::SetModelPath(std::string modelPath)
   {
      mModelPath = modelPath;
   }

   void CPolyMesh::SetTexturePath(std::string texturePath)
   {
      mTexturePath = texturePath;
   }
}