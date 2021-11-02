#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"

// Storage buffer test
struct GeometryVertex
{
   GeometryVertex(glm::vec4 _pos, glm::vec4 _normal)
      : pos(_pos), normal(_normal) {
   }

   glm::vec4 pos;
   glm::vec4 normal;
};

class VertexSSBO : public Utopian::Vk::ShaderBuffer
{
public:
   virtual void UpdateMemory(VkDevice device)
   {
      uint8_t* mapped;
      uint32_t size = (uint32_t)vertices.size() * sizeof(GeometryVertex);
      mBuffer->MapMemory((void**)&mapped);
      memcpy(mapped, vertices.data(), size);
      mBuffer->UnmapMemory();
   }

   virtual int GetSize()
   {
      return (int)vertices.size() * sizeof(GeometryVertex);
   }

   std::vector<GeometryVertex> vertices;
};

class Block
{
public:
   Block(Utopian::Vk::Device* device, glm::vec3 position, glm::vec3 color, uint32_t blockSize, float voxelSize);
   ~Block();

   Utopian::Vk::Buffer* GetVertexBuffer();

   Utopian::Vk::Buffer* vertexBuffer;
   VkDescriptorBufferInfo bufferInfo;
   glm::vec3 position;
   glm::vec3 color;
   bool generated;
   bool modified;
   bool visible;
   uint32_t numVertices;
   float voxelSize;
};
