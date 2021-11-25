#pragma once
#include <glm/glm.hpp>
#include <string.h>
#include "core/components/Component.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"
#include "OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh"

typedef OpenMesh::PolyMesh_ArrayKernelT<> PolyMesh;

namespace Utopian
{
   class Actor;
   class CRenderable;

   class CPolyMesh : public Component
   {
   public:
      CPolyMesh(Actor* parent);
      CPolyMesh(Actor* parent, std::string modelName, std::string texturePath);
      ~CPolyMesh();

      void Update(double deltaTime) override;
      void OnCreated() override;
      void OnDestroyed() override;
      void PostInit() override;
      void SerializeData() override;

      void PreFrame();

      LuaPlus::LuaObject GetLuaObject() override;

      void GetSelectedFaceVertices(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);
      void GetSelectedEdgeVertices(glm::vec3& v0, glm::vec3& v1);
      glm::vec3 GetSelectedFaceNormal();
      glm::vec3 GetSelectedFaceCenter();
      bool ShouldRebuildBuffers();

      void MoveSelectedEdge(const glm::vec3& delta);
      void MoveSelectedFace(const glm::vec3& delta);
      void ScaleSelectedFace(float delta);
      void ExtrudeSelectedFace(float extrusion);
      void SelectFace(Ray ray);
      void WriteToFile(std::string file);
      void LoadFromFile(std::string file);

      std::string GetModelPath() const;
      std::string GetTexturePath() const;

      void SetModelName(std::string modelName);
      void SetTexturePath(std::string texturePath);

      // Type identification
      static uint32_t GetStaticType() {
         return Component::ComponentType::POLYMESH;
      }

      virtual uint32_t GetType() {
         return GetStaticType();
      }

   private:
      OpenMesh::SmartHalfedgeHandle GetClosestEdge(const OpenMesh::SmartFaceHandle& face, glm::vec3 point);
      void UpdateMeshBuffer();
      void CreateCube();
      std::vector<PolyMesh::VertexHandle> AddExtrusionVertices(float extrusion);
      OpenMesh::SmartFaceHandle AddExtrusionFaces(std::vector<PolyMesh::VertexHandle>& vertices);

   private:
      PolyMesh mPolyMesh;
      OpenMesh::SmartFaceHandle mSelectedFace;
      OpenMesh::SmartHalfedgeHandle mSelectedEdge;
      bool mRebuildMeshBuffer = false;
      std::string mModelName;
      std::string mTexturePath;

      // Todo: "Fix component PostInit order #120"
      bool mFirstFrame = true;
   };
}
