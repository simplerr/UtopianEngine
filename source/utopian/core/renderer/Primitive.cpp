#include <vulkan/vulkan_core.h>
#include <glm/gtc/matrix_transform.hpp>
#include "core/ModelLoader.h"
#include "core/renderer/Primitive.h"
#include "vulkan/handles/Device.h"
#include "vulkan/Debug.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/Texture.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/CommandBuffer.h"

namespace Utopian
{
   Primitive::Primitive()
   {
   }

   Primitive::~Primitive()
   {
   }

   void Primitive::AddVertex(Vk::Vertex vertex)
   {
      vertices.push_back(vertex);
   }

   void Primitive::AddVertex(glm::vec3 pos)
   {
      AddVertex(Vk::Vertex(pos));
   }

   void Primitive::AddLine(uint32_t v1, uint32_t v2)
   {
      indices.push_back(v1);
      indices.push_back(v2);
   }

   void Primitive::AddTriangle(uint32_t v1, uint32_t v2, uint32_t v3)
   {
      indices.push_back(v1);
      indices.push_back(v2);
      indices.push_back(v3);
   }

   void Primitive::AddQuad(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4)
   {
      indices.push_back(v1);
      indices.push_back(v2);
      indices.push_back(v3);
      indices.push_back(v4);
   }

   void Primitive::BuildBuffers(Vk::Device* device)
   {
      uint32_t vertexBufferSize = GetNumVertices() * sizeof(Vk::Vertex);
      uint32_t indexBufferSize = GetNumIndices() * sizeof(uint32_t);

      Vk::BUFFER_CREATE_INFO stagingVertexCI;
      stagingVertexCI.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      stagingVertexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      stagingVertexCI.data = vertices.data();
      stagingVertexCI.size = vertexBufferSize;
      stagingVertexCI.name = "Staging Vertex buffer: " + mDebugName;
      Vk::Buffer vertexStaging = Vk::Buffer(stagingVertexCI, device);

      Vk::BUFFER_CREATE_INFO vertexCI;
      vertexCI.usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      vertexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      vertexCI.data = nullptr;
      vertexCI.size = vertexBufferSize;
      vertexCI.name = "Vertex buffer: " + mDebugName;
      mVertexBuffer = std::make_shared<Vk::Buffer>(vertexCI, device);

      // Copy from host visible to device local memory
      Vk::CommandBuffer cmdBuffer = Vk::CommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
      vertexStaging.Copy(&cmdBuffer, mVertexBuffer.get());
      cmdBuffer.Flush();

      if (GetNumIndices() > 0)
      {
         Vk::BUFFER_CREATE_INFO stagingIndexCI;
         stagingIndexCI.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
         stagingIndexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
         stagingIndexCI.data = indices.data();
         stagingIndexCI.size = indexBufferSize;
         stagingIndexCI.name = "Staging Index buffer: " + mDebugName;
         Vk::Buffer indexStaging = Vk::Buffer(stagingIndexCI, device);

         Vk::BUFFER_CREATE_INFO indexCI;
         indexCI.usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
         indexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
         indexCI.data = nullptr;
         indexCI.size = indexBufferSize;
         indexCI.name = "Index buffer: " + mDebugName;
         mIndexBuffer = std::make_shared<Vk::Buffer>(indexCI, device);

         cmdBuffer.Begin();
         indexStaging.Copy(&cmdBuffer, mIndexBuffer.get());
         cmdBuffer.Flush();
      }

      mBoundingBox.Init(vertices);
   }

   uint32_t Primitive::GetNumVertices() const
   {
      return vertices.size();
   }

   uint32_t Primitive::GetNumIndices() const
   {
      return indices.size();
   }

   Vk::Buffer* Primitive::GetVertxBuffer()
   {
      return mVertexBuffer.get();
   }

   Vk::Buffer* Primitive::GetIndexBuffer()
   {
      return mIndexBuffer.get();
   }

   BoundingBox Primitive::GetBoundingBox()
   {
      return mBoundingBox;
   }

   void Primitive::SetDebugName(std::string debugName)
   {
      mDebugName = debugName;
   }
}
