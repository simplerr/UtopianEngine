#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "core/renderer/Primitive.h"
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

      void AddMesh(Primitive* mesh);
      void Init(Device* device);    // Gets called in ModelLoader::LoadModel()

      int GetNumIndices();
      int GetNumVertics();
      BoundingBox GetBoundingBox();

      std::vector<Primitive*> mMeshes;
   private:
      uint32_t mIndicesCount;
      uint32_t mVerticesCount;

      BoundingBox mBoundingBox;
   };
}  // VulkanLib namespace