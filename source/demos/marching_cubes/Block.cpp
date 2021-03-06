#include "Block.h"

Block::Block(Utopian::Vk::Device* device, glm::vec3 position, glm::vec3 color, uint32_t blockSize, float voxelSize)
{
   this->position = position;
   this->color = color;
   this->generated = false;
   this->modified = false;
   this->voxelSize = voxelSize;
   this->numVertices = 0;
   this->visible = true;

   uint32_t size = blockSize * blockSize * blockSize * 5 * sizeof(glm::vec3);
   Utopian::Vk::BUFFER_CREATE_INFO createInfo;
   createInfo.data = nullptr;
   createInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
   createInfo.usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
   createInfo.size = size;
   createInfo.name = "Marching cubes block buffer";

   vertexBuffer = new Utopian::Vk::Buffer(createInfo,device);
   bufferInfo.buffer = vertexBuffer->GetVkHandle();
   bufferInfo.range = size;
   bufferInfo.offset = 0;
}

Block::~Block()
{
   delete vertexBuffer;
}

Utopian::Vk::Buffer* Block::GetVertexBuffer()
{
   return vertexBuffer;
}
