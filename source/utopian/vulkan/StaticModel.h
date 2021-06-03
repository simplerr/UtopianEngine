#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "vulkan/Mesh.h"
#include "utility/math/BoundingBox.h"
#include "Vertex.h"
#include "vulkan/VulkanPrerequisites.h"

namespace Utopian::Vk
{
   class StaticModel
   {
   public:
      StaticModel();
      ~StaticModel();

      void AddMesh(Mesh* mesh);
      void Init(Device* device);    // Gets called in ModelLoader::LoadModel()

      int GetNumIndices();
      int GetNumVertics();
      BoundingBox GetBoundingBox();

      std::vector<Mesh*> mMeshes;
   private:
      uint32_t mIndicesCount;
      uint32_t mVerticesCount;

      BoundingBox mBoundingBox;
   };
}  // VulkanLib namespace