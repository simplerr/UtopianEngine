#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <vulkan/Mesh.h>
#include "core/World.h"
#include "OpenMesh/Core/IO/MeshIO.hh"
#include "OpenMesh/Core/Geometry/Vector11T.hh"
#include "OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh"

typedef OpenMesh::PolyMesh_ArrayKernelT<> PolyMesh;

namespace Utopian
{
   class World;
   class Actor;

   class PrototypeTool
   {
      public:
         PrototypeTool();
         ~PrototypeTool();

         void Update(World* world, Actor* selectedActor);
         void PreFrame();
         void RenderUi();

         void ActorSelected(Actor* actor, glm::vec3 normal);
      private:
         Actor* AddBox(glm::vec3 position, std::string texture);

         // OpenMesh experimentation
         PolyMesh CreatePolyMeshCube();
         OpenMesh::SmartFaceHandle AddFaceExtrude(std::vector<PolyMesh::VertexHandle>& vhandle);
         void WriteToFile(const PolyMesh& mesh, std::string file);
         void UpdateMeshBuffer(Utopian::Vk::Mesh* mesh, const PolyMesh& polyMesh);
         OpenMesh::SmartHalfedgeHandle GetClosestEdge(const OpenMesh::SmartFaceHandle& face, glm::vec3 point);

         PolyMesh mPolyMesh;

      private:
         const SceneLayer PrototypeBoxSceneLayer = 1u;
         OpenMesh::SmartFaceHandle mSelectedFace;
         OpenMesh::SmartHalfedgeHandle mSelectedEdge;
         bool mSelected = false;
         Actor* mSelectedActor = nullptr;

         Utopian::Vk::StaticModel* mSelectedModel = nullptr;
         bool mRebuildMeshBuffer = false;
   };
}