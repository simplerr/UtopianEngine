#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/Buffer.h"
#include "utility/math/BoundingBox.h"
#include "utility/Common.h"

// Todo: move these
#define DEFAULT_NORMAL_MAP_TEXTURE "data/textures/flat_normalmap.png"
#define DEFAULT_SPECULAR_MAP_TEXTURE "data/textures/default_specular_map.png"

namespace Utopian
{
   class Primitive
   {
   public:
      Primitive(Vk::Device* device);
      ~Primitive();

      void AddVertex(Vk::Vertex vertex);
      void AddVertex(glm::vec3 pos);
      void AddLine(uint32_t v1, uint32_t v2);
      void AddTriangle(uint32_t v1, uint32_t v2, uint32_t v3);
      void AddQuad(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4);
      void BuildBuffers(Vk::Device* device);

      uint32_t GetNumIndices() const;
      uint32_t GetNumVertices() const;

      Vk::Buffer* GetVertxBuffer();
      Vk::Buffer* GetIndexBuffer();
      BoundingBox GetBoundingBox();

      void SetDebugName(std::string debugName);

      std::vector<Vk::Vertex> vertices;
      std::vector<unsigned int> indices;

   private:
      SharedPtr<Vk::Buffer> mVertexBuffer;
      SharedPtr<Vk::Buffer> mIndexBuffer;
      Vk::Device* mDevice;
      BoundingBox mBoundingBox;
      std::string mDebugName = "unnamed";
   };
}
