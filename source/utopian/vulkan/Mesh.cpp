#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/handles/Device.h"
#include "vulkan/Debug.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/Texture.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/CommandBuffer.h"
#include "Mesh.h"

namespace Utopian::Vk
{
   Mesh::Mesh(Device* device)
   {
      mDevice = device;
      mDiffuseTexture = nullptr;
      mNormalTexture = nullptr;
      mSpecularTexture = nullptr;
   }

   Mesh::~Mesh()
   {
   }

   void Mesh::AddVertex(Vertex vertex)
   {
      vertexVector.push_back(vertex);
   }

   void Mesh::AddVertex(float x, float y, float z)
   {
      AddVertex(Vertex(x, y, z));
   }

   void Mesh::AddLine(uint32_t v1, uint32_t v2)
   {
      indexVector.push_back(v1);
      indexVector.push_back(v2);
   }

   void Mesh::AddTriangle(uint32_t v1, uint32_t v2, uint32_t v3)
   {
      indexVector.push_back(v1);
      indexVector.push_back(v2);
      indexVector.push_back(v3);
   }

   void Mesh::AddQuad(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4)
   {
      indexVector.push_back(v1);
      indexVector.push_back(v2);
      indexVector.push_back(v3);
      indexVector.push_back(v4);
   }

   void Mesh::BuildBuffers(Device* device)
   {
      mVerticesCount = (uint32_t)vertexVector.size();
      mIndicesCount = (uint32_t)indexVector.size();

      uint32_t vertexBufferSize = mVerticesCount * sizeof(Vertex);
      uint32_t indexBufferSize = mIndicesCount * sizeof(uint32_t);

      // Host visible staging buffers
      BUFFER_CREATE_INFO stagingVertexCI;
      stagingVertexCI.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      stagingVertexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      stagingVertexCI.data = vertexVector.data();
      stagingVertexCI.size = vertexBufferSize;
      stagingVertexCI.name = "Staging Vertex buffer: " + mDebugName;
      Buffer vertexStaging = Buffer(stagingVertexCI, device);

      BUFFER_CREATE_INFO stagingIndexCI;
      stagingIndexCI.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      stagingIndexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      stagingIndexCI.data = indexVector.data();
      stagingIndexCI.size = indexBufferSize;
      stagingIndexCI.name = "Staging Index buffer: " + mDebugName;
      Buffer indexStaging = Buffer(stagingIndexCI, device);

      // Device local target buffers
      BUFFER_CREATE_INFO vertexCI;
      vertexCI.usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      vertexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      vertexCI.data = nullptr;
      vertexCI.size = vertexBufferSize;
      vertexCI.name = "Vertex buffer: " + mDebugName;
      mVertexBuffer = std::make_shared<Buffer>(vertexCI, device);

      BUFFER_CREATE_INFO indexCI;
      indexCI.usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      indexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      indexCI.data = nullptr;
      indexCI.size = indexBufferSize;
      indexCI.name = "Index buffer: " + mDebugName;
      mIndexBuffer = std::make_shared<Buffer>(indexCI, device);

      // Copy from host visible to device local memory
      CommandBuffer cmdBuffer = CommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

      vertexStaging.Copy(&cmdBuffer, mVertexBuffer.get());
      indexStaging.Copy(&cmdBuffer, mIndexBuffer.get());

      cmdBuffer.Flush();
   }

   void Mesh::BuildBuffers(const std::vector<Vertex>& vertices, std::vector<uint32_t>)
   {
   }

   void Mesh::SetTexture(SharedPtr<Vk::Texture> texture)
   {
      mDiffuseTexture = texture;
      CreateDescriptorSets(gModelLoader().GetMeshTextureDescriptorSetLayout(), gModelLoader().GetMeshTextureDescriptorPool());
   }

   void Mesh::SetSpecularTexture(SharedPtr<Vk::Texture> texture)
   {
      mSpecularTexture = texture;
      CreateDescriptorSets(gModelLoader().GetMeshTextureDescriptorSetLayout(), gModelLoader().GetMeshTextureDescriptorPool());
   }

   VkDescriptorSet Mesh::GetTextureDescriptorSet()
   {
      return mTextureDescriptorSet->GetVkHandle();
   }

   void Mesh::CreateDescriptorSets(SharedPtr<DescriptorSetLayout> descriptorSetLayout, SharedPtr<DescriptorPool> descriptorPool)
   {
      mTextureDescriptorSet = std::make_shared<DescriptorSet>(mDevice, descriptorSetLayout.get(), descriptorPool.get());
      mTextureDescriptorSet->BindCombinedImage(0, mDiffuseTexture->GetDescriptor());
      mTextureDescriptorSet->BindCombinedImage(1, mNormalTexture->GetDescriptor());
      mTextureDescriptorSet->BindCombinedImage(2, mSpecularTexture->GetDescriptor());
      mTextureDescriptorSet->UpdateDescriptorSets();
   }

   BoundingBox Mesh::GetBoundingBox()
   {
      return mBoundingBox;
   }

   uint32_t Mesh::GetNumIndices()
   {
      return mIndicesCount;
   }

   Buffer* Mesh::GetVertxBuffer()
   {
      return mVertexBuffer.get();
   }

   Buffer* Mesh::GetIndexBuffer()
   {
      return mIndexBuffer.get();
   }

   Texture* Mesh::GetDiffuseTexture()
   {
       return mDiffuseTexture.get(); 
   }

   Texture* Mesh::GetNormalTexture()
   {
      return mNormalTexture.get();
   }

   Texture* Mesh::GetSpecularTexture()
   {
      return mSpecularTexture.get();
   };

   std::vector<Texture*> Mesh::GetTextures()
   {
      std::vector<Texture*> textures;
      textures.push_back(mDiffuseTexture.get());
      textures.push_back(mNormalTexture.get());
      textures.push_back(mSpecularTexture.get());

      return textures;
   }

   void Mesh::LoadTextures(std::string diffusePath, std::string normalPath, std::string specularPath)
   {
      mDiffuseTexture = gTextureLoader().LoadTexture(diffusePath);
      mNormalTexture = gTextureLoader().LoadTexture(normalPath);
      mSpecularTexture = gTextureLoader().LoadTexture(specularPath);
      CreateDescriptorSets(gModelLoader().GetMeshTextureDescriptorSetLayout(), gModelLoader().GetMeshTextureDescriptorPool());
   }

   void Mesh::SetDebugName(std::string debugName)
   {
      mDebugName = debugName;
   }
}
